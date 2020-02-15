// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/Value.h"

#include <limits>
#include <cassert>

namespace blackboard {

template <typename T>
constexpr bool Value::HasType() const noexcept {
    return std::holds_alternative<T>(value);
}

template <typename T>
constexpr T Value::Get() const noexcept {
    return std::get<T>(value);
}

constexpr bool Value::IsUndefined() const noexcept {
    return HasType<Value::UndefinedType>();
}

constexpr bool Value::IsNumber() const noexcept {
    return HasType<Value::NumberType>();
}

constexpr bool Value::IsString() const noexcept {
    return HasType<Value::StringType>();
}

constexpr bool Value::IsBoolean() const noexcept {
    return HasType<Value::BooleanType>();
}

constexpr bool Value::IsReference() const noexcept {
    return HasType<Value::ReferenceType>();
}

constexpr bool Value::IsObject() const noexcept {
    return HasType<Value::ObjectType>();
}

constexpr Value::NumberType Value::GetNumber() const noexcept {
    return Get<Value::NumberType>();
}

constexpr Value::StringType Value::GetString() const noexcept {
    return Get<Value::StringType>();
}

constexpr Value::BooleanType Value::GetBoolean() const noexcept {
    return Get<Value::BooleanType>();
}

constexpr Value::ReferenceType Value::GetReference() const noexcept {
    return Get<Value::ReferenceType>();
}

constexpr Value::ObjectType Value::GetObject() const noexcept {
    return Get<Value::ObjectType>();
}

//--------------------------------------------------------------------------------------------------

Value::Value() : value(UndefinedType()) {}

Value::Value(const Value& from) {
    InitializeFrom(from);
}

Value::Value(Value&& from) noexcept {
    InitializeFrom(from);
    from.value = UndefinedType();
}

Value::~Value() {
    Clear();
}

//--------------------------------------------------------------------------------------------------

Value::Value(double from) {
    FromNumber(from);
}

Value::Value(std::string_view from) {
    FromString(from);
}

Value::Value(bool from) {
    FromBoolean(from);
}

Value::Value(void* from) {
    FromReference(from);
}

Value::Value(const Object& from) {
    FromObject(from);
}

//--------------------------------------------------------------------------------------------------

Value& Value::operator=(const Value& from) noexcept {
    if (this != &from) {
        this->~Value();
        new (this) Value(from);
    }
    return *this;
}

Value& Value::operator=(Value&& from) noexcept {
    this->~Value();
    return *new (this) Value(from);
}

bool Value::operator==(const Value& other) const noexcept {
    if (this == &other) {
        return true;
    } else if (value.index() != other.value.index()) {
        return false;
    } else {
        if (IsString()) {
            return *GetString() == *other.GetString();
        } else if (IsObject()) {
            return *GetObject() == *other.GetObject();
        } else {
            return value == other.value;
        }
    }
}

bool Value::operator!=(const Value& other) const noexcept {
    return !(*this == other);
}

std::optional<Value> Value::operator[](const Value& key) const {
    return ToObject().GetValue(key);
}

//--------------------------------------------------------------------------------------------------

Value::operator bool() const noexcept {
    switch (value.index()) {
    case undefinedTypeIndex:
        return false;
    case numberTypeIndex:
        return GetNumber() != 0;
    case stringTypeIndex:
        return !GetString()->empty();
    case booleanTypeIndex:
        return GetBoolean();
    case referenceTypeIndex:
        return reinterpret_cast<unsigned long>(GetReference()) != 0;
    case objectTypeIndex:
        return true;
    default:
        assert("Unhandled value type!" && false);
        return false;
    }
}

//--------------------------------------------------------------------------------------------------

void Value::FromNumber(double from) {
    Clear();
    value = from;
}

void Value::FromString(std::string_view from) {
    Clear();
    value = new std::string(from);
}

void Value::FromBoolean(bool from) {
    Clear();
    value = from;
}

void Value::FromReference(void* from) {
    Clear();
    value = from;
}

void Value::FromObject(const Object& from) {
    Clear();
    value = new Object(from);
}

//--------------------------------------------------------------------------------------------------

double Value::ToNumber() const {
    assert(IsNumber());
    return GetNumber();
}

std::string& Value::ToString() const {
    assert(IsString());
    return *GetString();
}

bool Value::ToBoolean() const {
    assert(IsBoolean());
    return GetBoolean();
}

void* Value::ToReference() const {
    assert(IsReference());
    return GetReference();
}

Object& Value::ToObject() const {
    assert(IsObject());
    return *GetObject();
}

//--------------------------------------------------------------------------------------------------

std::string Value::GetType() const {
    switch(value.index()) {
    case undefinedTypeIndex:
        return "Undefined";
    case numberTypeIndex:
        return "Number";
    case booleanTypeIndex:
        return "Boolean";
    case referenceTypeIndex:
        return "Reference";
    case stringTypeIndex:
        return "String";
    case objectTypeIndex:
        return "Object";
    default:
        assert("Unhandled value type!" && false);
        return "";
    }
}

//--------------------------------------------------------------------------------------------------

void Value::InitializeFrom(const Value& from) {
    if (from.IsString()) {
        value = new std::string(*from.GetString());
    } else if (from.IsObject()) {
        value = new Object(*from.GetObject());
    } else {
        value = from.value;
    }
}

void Value::Clear() {
    if (IsString()) {
        delete GetString();
    } else if (IsObject()) {
        delete GetObject();
    }
    value = UndefinedType();
}

//--------------------------------------------------------------------------------------------------

std::size_t ValueHash::operator()(const Value& value) const {
    if (value.IsUndefined()) {
        return std::numeric_limits<std::size_t>::min();
    } else if (value.IsNumber()) {
        return std::hash<double>{}(value.GetNumber());
    } else if (value.IsBoolean()) {
        return std::hash<bool>{}(value.GetBoolean());
    } else if (value.IsReference()) {
        return std::hash<unsigned long>{}(reinterpret_cast<unsigned long>(value.GetReference()));
    } else if (value.IsString()) {
        return std::hash<std::string>{}(*value.GetString());
    } else if (value.IsObject()) {
        return std::hash<unsigned long>{}(reinterpret_cast<unsigned long>(value.GetObject()));
    } else {
        assert("Unhandled value type!" && false);
        return std::numeric_limits<std::size_t>::max();
    }
}

} // namespace blackboard
