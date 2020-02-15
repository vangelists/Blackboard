// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#pragma once

#include "Blackboard/Object.h"
#include "Blackboard/Utilities.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace blackboard {

class Value {
    friend struct ValueHash;

public:
    Value();
    Value(const Value& from);
    Value(Value&& from) noexcept;
    ~Value();

    explicit Value(double from);
    explicit Value(std::string_view from);
    explicit Value(bool from);
    explicit Value(void* from);
    explicit Value(const Object& from);

    Value& operator=(const Value& from) noexcept;
    Value& operator=(Value&& from) noexcept;
    bool operator==(const Value& other) const noexcept;
    bool operator!=(const Value& other) const noexcept;
    std::optional<Value> operator[](const Value& key) const;

    explicit operator bool() const noexcept;

    void FromNumber(double from);
    void FromString(std::string_view from);
    void FromBoolean(bool from);
    void FromReference(void* from);
    void FromObject(const Object& from);

    double ToNumber() const;
    std::string& ToString() const;
    bool ToBoolean() const;
    void* ToReference() const;
    Object& ToObject() const;

    std::string GetType() const;

private:
    using UndefinedType = std::monostate;
    using NumberType = double;
    using StringType = std::string*;
    using BooleanType = bool;
    using ReferenceType = void*;
    using ObjectType = Object*;

    using ValueHolder = std::variant<UndefinedType, NumberType, StringType, BooleanType,
                                     ReferenceType, ObjectType>;

    void InitializeFrom(const Value& from);
    void Clear();

    template <typename T>
    constexpr bool HasType() const noexcept;

    constexpr bool IsUndefined() const noexcept;
    constexpr bool IsNumber() const noexcept;
    constexpr bool IsString() const noexcept;
    constexpr bool IsBoolean() const noexcept;
    constexpr bool IsReference() const noexcept;
    constexpr bool IsObject() const noexcept;

    template <typename T>
    constexpr T Get() const noexcept;

    constexpr NumberType GetNumber() const noexcept;
    constexpr StringType GetString() const noexcept;
    constexpr BooleanType GetBoolean() const noexcept;
    constexpr ReferenceType GetReference() const noexcept;
    constexpr ObjectType GetObject() const noexcept;

    static constexpr auto undefinedTypeIndex = variant_index<ValueHolder, UndefinedType>();
    static constexpr auto numberTypeIndex = variant_index<ValueHolder, NumberType>();
    static constexpr auto stringTypeIndex = variant_index<ValueHolder, StringType>();
    static constexpr auto booleanTypeIndex = variant_index<ValueHolder, BooleanType>();
    static constexpr auto referenceTypeIndex = variant_index<ValueHolder, ReferenceType>();
    static constexpr auto objectTypeIndex = variant_index<ValueHolder, ObjectType>();

    ValueHolder value;
};

struct ValueHash {
    std::size_t operator()(const Value& value) const;
};

} // namespace blackboard
