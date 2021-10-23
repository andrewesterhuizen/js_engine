#include "object.h"

namespace interpreter::object {

bool ObjectManager::is_undefined(Object* value) {
    return value->type() == ObjectType::Undefined;
}

Object* ObjectManager::new_object() {
    // just leak for now
    return new Object(ObjectType::Object);
}

Function* ObjectManager::new_function() {
    // just leak for now
    return new Function();
}

Array* ObjectManager::new_array() {
    // just leak for now
    return new Array();
}

Number* ObjectManager::new_number(double value) {
    // just leak for now
    return new Number(value);
}

String* ObjectManager::new_string(std::string value) {
    // just leak for now
    return new String(value);
}

Boolean* ObjectManager::new_boolean(bool value) {
    // just leak for now
    return new Boolean(value);
}

Undefined* ObjectManager::new_undefined() {
    // just leak for now
    return new Undefined();
}

Number* Object::as_number() {
    assert(type() == ObjectType::Number);
    return static_cast<Number*>(this);
}

}