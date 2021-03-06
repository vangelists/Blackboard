// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/Object.h"
#include "Blackboard/Value.h"

namespace blackboard {

Object::Object() : values(std::make_unique<Values>()) {}

Object::Object(const Object& from) : values(std::make_unique<Values>(*from.values)) {}

Object::Object(Object&& from) noexcept = default;

Object::~Object() = default;

//--------------------------------------------------------------------------------------------------

Object& Object::operator=(const Object& from) {
    if (this != &from) {
        this->~Object();
        new (this) Object(from);
    }
    return *this;
}

Object& Object::operator=(Object&& from) noexcept {
    if (this != &from) {
        this->~Object();
        return *new (this) Object(from);
    }
    return *this;
}

bool Object::operator==(const Object& other) const {
    if (this == &other) {
        return true;
    }
    return *values == *other.values;
}

bool Object::operator!=(const Object& other) const {
    return !(*this == other);
}

std::optional<Value> Object::operator[](const Value& key) const {
    return GetValue(key);
}

//--------------------------------------------------------------------------------------------------

std::optional<Value> Object::GetValue(const Value& key) const {
    if (auto iterator = values->find(key); iterator != values->end()) {
        return iterator->second;
    }
    return {};
}

Object& Object::AddValue(const Value& key, const Value& value) {
    if (auto iterator = values->find(key); iterator != values->end()) {
        iterator->second = value;
    } else {
        values->emplace(key, value);
    }
    return *this;
}

Object& Object::RemoveValue(const Value& key) {
    if (auto iterator = values->find(key); iterator != values->end()) {
        values->erase(key);
    }
    return *this;
}

} // namespace blackboard
