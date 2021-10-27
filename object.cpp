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

object::Value* ObjectManager::get_variable(std::string name) {
    auto i = current_scope_index;

    while (i >= 0) {
        auto value = scopes[i].get_variable(name);
        if (value != nullptr) {
            return value;
        }

        i--;
    }

    return nullptr;
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

void Value::register_native_method(ObjectManager &object_manager, std::string name, native_function_handler handler) {
    auto func_value = object_manager.new_function();
    auto func = func_value->function();
    func->is_builtin = true;
    func->builtin_func = handler;
    properties[name] = func_value;
}

Value* Value::get_property(ObjectManager &object_manager, std::string name) {
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

    if (!prototype.has_value()) {
        return nullptr;
    }

    auto proto = object_manager.get_variable(prototype.value());
    if (proto == nullptr) {
        return nullptr;
    }

    return proto->get_property(object_manager, name);
}

Value* Value::get_property(ObjectManager &object_manager, int index) {
    if (type == Type::Array) {
        auto a = array();
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

}