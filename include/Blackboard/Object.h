// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

namespace blackboard {

class Value;
struct ValueHash;

class Object {
public:
    Object();
    Object(const Object& from);
    Object(Object&& from) noexcept;
    ~Object();

    Object& operator=(const Object& from);
    Object& operator=(Object&& from) noexcept;
    bool operator==(const Object& other) const;
    bool operator!=(const Object& other) const;
    std::optional<Value> operator[](const Value& key) const;

    std::optional<Value> GetValue(const Value& key) const;
    Object& AddValue(const Value& key, const Value& value);
    Object& RemoveValue(const Value& key);

private:
    using Values = std::unordered_map<const Value, Value, ValueHash>;

    std::unique_ptr<Values> values;
};

} // namespace blackboard
