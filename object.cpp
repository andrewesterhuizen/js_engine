#include "object.h"

namespace interpreter::object {

bool ObjectManager::is_undefined(Object* value) {
    return value->type() == ObjectType::Undefined;
}

Object* ObjectManager::new_object() {
    return allocate<Object>(ObjectType::Object);
}

Function* ObjectManager::new_function() {
    return allocate<Function>();
}

Array* ObjectManager::new_array() {
    return allocate<Array>();
}

Number* ObjectManager::new_number(double value) {
    return allocate<Number>(value);
}

String* ObjectManager::new_string(std::string value) {
    return allocate<String>(value);
}

Boolean* ObjectManager::new_boolean(bool value) {
    return allocate<Boolean>(value);
}

Undefined* ObjectManager::new_undefined() {
    return allocate<Undefined>();
}

Number* Object::as_number() {
    assert(type() == ObjectType::Number);
    return static_cast<Number*>(this);
}

void Object::register_native_method(std::string name, std::function<Object*(std::vector<Object*>)> handler) {
    // TODO: don't leak
    auto func = new Function();
    func->is_builtin = true;
    func->builtin_func = handler;
    properties[name] = func;
}

Scope* ObjectManager::current_scope() {
    return &scopes[current_scope_index];
}

void ObjectManager::push_scope() {
    scopes.push_back(Scope{});
    current_scope_index++;
}

void ObjectManager::pop_scope() {
    scopes.pop_back();
    current_scope_index--;
}


object::Object* ObjectManager::get_variable(std::string name) {
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

object::Object* ObjectManager::declare_variable(std::string name, object::Object* value) {
    return current_scope()->set_variable(name, value);
}

object::Object* ObjectManager::set_variable(std::string name, object::Object* value) {
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
            objects[obj] = true;
            for (auto p: obj->properties) {
                objects[p.second] = true;
            }
        }
    }

    // get list of unmarked objects
    std::vector<Object*> to_remove;
    for (auto o: objects) {
        if (!o.second) {
            to_remove.push_back(o.first);
        }
    }

    // remove unmarked objects
    for (auto o: to_remove) {
        delete o;
        objects.erase(o);
        objects_collected++;
    }

    for (auto &c: objects) {
        c.second = false;
    }
}

template<typename T, typename... Args>
T* ObjectManager::allocate(Args &&... args) {
    if (objects.size() > gc_threshold) {
        collect_garbage();
    }

    auto o = new T(std::forward<Args>(args)...);
    objects[o] = false;
    return o;
}

}