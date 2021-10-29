#include "object.h"

namespace interpreter::object {

ObjectManager::Scope* ObjectManager::current_scope() {
    return &scopes[current_scope_index];
}

void ObjectManager::push_scope(Value* context) {
    scopes.push_back(Scope{*this, context});
    current_scope_index++;
}

void ObjectManager::pop_scope() {
    scopes.pop_back();
    current_scope_index--;
}

std::optional<object::Value*> ObjectManager::get_variable(std::string name) {
    auto i = current_scope_index;

    while (i >= 0) {
        auto var = scopes[i].get_variable(name);
        if (var.has_value()) {
            return var.value();
        }

        i--;
    }

    return {};
}

object::Value* ObjectManager::declare_variable(std::string name, object::Value* value) {
    return current_scope()->set_variable(name, value);
}

object::Value* ObjectManager::set_variable(std::string name, object::Value* value) {
    auto var = current_scope()->get_variable(name);

    // undeclared so set in top level scope, closest thing we have to "global" right now
    if (var == nullptr) {
        return scopes[0].set_variable(name, value);
    }

    return current_scope()->set_variable(name, value);
}
void ObjectManager::collect_garbage() {
    gc_amount++;

    // mark referenced objects
    for (auto i = 0; i <= current_scope_index; i++) {
        for (auto v: scopes[i].variables) {
            auto obj = v.second;
            objects[obj].is_referenced = true;
            for (auto p: obj->properties) {
                objects[p.second].is_referenced = true;
            }
        }
    }


    // get list of unmarked objects
    std::vector<Value*> to_remove;
    for (auto o: objects) {
        auto obj = o.second;
        if (is_value_still_in_scope(obj.value)) {
            continue;
        }

        if (!obj.is_referenced) {
            to_remove.push_back(o.first);
        }
    }

    // remove unmarked objects
    for (auto o: to_remove) {
        delete o;
        objects.erase(o);
        objects_collected++;
    }

    // reset objects
    for (auto &c: objects) {
        c.second.is_referenced = false;
    }
}

template<typename T, typename... Args>
T* ObjectManager::allocate(Args &&... args) {
    if (objects.size() > gc_threshold) {
        // TODO: fix garbage collection
        // collect_garbage();
    }

    auto o = new T(std::forward<Args>(args)...);
    objects[o] = Cell{o, true};

    if (scopes.size() > 0) {
        current_scope()->values_in_scope.insert(o);
    }
    return o;
}

ObjectManager::Scope* ObjectManager::global_scope() {
    return &scopes[0];
}

Value* ObjectManager::global_object() {
    return global;
}

Value* Value::register_native_method(ObjectManager &object_manager, std::string name, native_function_handler handler) {
    auto func_value = object_manager.new_function(name);
    auto func = func_value->function();
    func->is_builtin = true;
    func->builtin_func = handler;
    properties[name] = func_value;
    return func_value;
}

std::optional<Value*> Value::get_property(ObjectManager &object_manager, std::string name) {
    if (type == Type::Array) {
        // TODO: built in properties like this need to be generalised
        if (name == "length") {
            auto a = array();
            return object_manager.new_number(a->elements.size());
        }
    }

    if (auto entry = properties.find(name); entry != properties.end()) {
        return entry->second;
    }

    auto proto = properties.find("__proto__");
    if (proto == properties.end()) {
        return {};
    }

    return proto->second->get_property(object_manager, name);
}

std::optional<Value*> Value::get_property(ObjectManager &object_manager, int index) {
    if (type == Type::Array) {
        auto a = array();
        if(index >= array()->elements.size()) {
            return object_manager.new_undefined();
        }

        return a->elements.at(index);
    }

    auto name = std::to_string(index);
    return get_property(object_manager, name);
}

Value* Value::set_property(std::string name, Value* value) {
    properties[name] = value;
    return value;
}

Value* Value::set_property(int index, Value* value) {
    if (type == Type::Array) {
        auto a = array();

        if (index > a->elements.size()) {
            a->elements.resize(index + 1);
        }

        a->elements[index] = value;
        return value;
    }

    auto name = std::to_string(index);
    return set_property(name, value);
}

Value* ValueFactory::number(ObjectManager &om, Value* value, double v) {
    value->type = Value::Type::Number;
    value->value = v;
    auto prototype = om.global_scope()->get_variable("Number");
    assert(prototype.has_value());
    value->set_property("__proto__", prototype.value());
    return value;
}
Value* ValueFactory::string(ObjectManager &om, Value* value, std::string v) {
    value->type = Value::Type::String;
    value->value = v;
    auto prototype = om.global_scope()->get_variable("String");
    assert(prototype.has_value());
    value->set_property("__proto__", prototype.value());
    return value;
}

Value* ValueFactory::boolean(ObjectManager &om, Value* value, bool v) {
    value->type = Value::Type::Boolean;
    value->value = v;
    auto prototype = om.global_scope()->get_variable("Boolean");
    assert(prototype.has_value());
    value->set_property("__proto__", prototype.value());
    return value;
}

Value* ValueFactory::null(ObjectManager &om, Value* value) {
    value->type = Value::Type::Null;
    auto prototype = om.global_scope()->get_variable("Object");
    assert(prototype.has_value());
    value->set_property("__proto__", prototype.value());
    return value;
}

Value* ValueFactory::array(ObjectManager &om, Value* value, std::optional<int> length) {
    return array(om, value, {}, length);
}

Value* ValueFactory::array(ObjectManager &om, Value* value, std::vector<Value*> v, std::optional<int> length) {
    value->type = Value::Type::Array;
    value->value = Value::Array{v};
    auto prototype = om.global_scope()->get_variable("Array");
    assert(prototype.has_value());
    value->set_property("__proto__", prototype.value());

    // this is a bit of a hack for now, arrays should support "holes",
    // so we don't need to allocate values for items that don't exist
    if(length.has_value()) {
        for(auto i = 0; i < length; i++) {
            value->array()->elements.push_back(om.new_undefined());
        }
    }

    return value;
}

Value* ValueFactory::object(ObjectManager &om, Value* value) {
    std::unordered_map<std::string, Value*> v;
    return object(om, value, v);
}

Value* ValueFactory::object(ObjectManager &om, Value* value, std::unordered_map<std::string, Value*> v) {
    value->type = Value::Type::Object;
    value->value = v;
    auto g = om.global_scope();
    if (g != nullptr) {
        auto prototype = om.global_scope()->get_variable("Object");
        if (prototype.has_value()) {
            assert(prototype.has_value());
            value->set_property("__proto__", prototype.value());
        }
    }
    return value;
}

Value* ValueFactory::function(ObjectManager &om, Value* value, std::optional<std::string> name) {
    value->type = Value::Type::Function;
    auto func = Value::Function{};
    func.name = name;
    value->value = func;

    auto prototype = om.new_object();
    prototype->set_property("constructor", value);
    value->set_property("prototype", prototype);

    // not sure if this one is correct
    auto proto = om.global_scope()->get_variable("Object");
    assert(proto.has_value());
    value->set_property("__proto__", proto.value());

    return value;
}

Value* ValueFactory::undefined(ObjectManager &om, Value* value) {
    value->type = Value::Type::Undefined;
    auto prototype = om.global_scope()->get_variable("Object");
    assert(prototype.has_value());
    value->set_property("__proto__", prototype.value());
    return value;
}


}