// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/Value.h"
#include "Blackboard/Object.h"

#include <functional>
#include <stdexcept>
#include <cstddef>

#include <catch.hpp>

using Value = blackboard::Value;
using ValueHash = blackboard::ValueHash;
using Object = blackboard::Object;

using namespace std::string_literals;

TEST_CASE("ValueHash", "[ValueTest]") {
    Value value;
    Value valueCopy = value;

    REQUIRE(ValueHash()(value) == ValueHash()(valueCopy));
}

TEST_CASE("FromNumber", "[ValueTest]") {
    Value value{};
    value.FromNumber(5);

    REQUIRE(value.ToNumber() == 5);
}

TEST_CASE("FromString", "[ValueTest]") {
    Value value{};
    value.FromString("Value::FromString() Test");

    REQUIRE(value.ToString() == "Value::FromString() Test");
}

TEST_CASE("FromBoolean", "[ValueTest]") {
    Value value{};
    value.FromBoolean(true);

    REQUIRE(value.ToBoolean());
}

TEST_CASE("FromReference", "[ValueTest]") {
    std::size_t thirteen = 13;
    Value value{};
    value.FromReference(&thirteen);

    REQUIRE(value.ToReference() == &thirteen);
    REQUIRE(*static_cast<std::size_t *>(value.ToReference()) == 13);
}

TEST_CASE("FromObject", "[ValueTest]") {
    Value stringValue{};
    stringValue.FromString("Thirteen");

    Value numberValue{};
    numberValue.FromNumber(13);

    Object object{};
    object.AddValue(stringValue, numberValue);
    object.AddValue(numberValue, stringValue);

    Value value{};
    value.FromObject(object);

    REQUIRE(value[stringValue]);
    REQUIRE(value[stringValue]->ToNumber() == 13);

    REQUIRE(value.ToObject().GetValue(numberValue));
    REQUIRE(value.ToObject().GetValue(numberValue)->ToString() == "Thirteen");
}

TEST_CASE("ValueGetType", "[ValueTest]") {
    using namespace std::string_literals;

    Value undefinedValue{};
    Value numberValue{13.0};
    Value stringValue{"Thirteen"s};
    Value booleanValue{true};
    Value referenceValue{reinterpret_cast<void*>(0xDEAFBEEF)};
    Value objectValue{Object{}};

    REQUIRE(undefinedValue.GetType() == "Undefined");
    REQUIRE(numberValue.GetType() == "Number");
    REQUIRE(stringValue.GetType() == "String");
    REQUIRE(booleanValue.GetType() == "Boolean");
    REQUIRE(referenceValue.GetType() == "Reference");
    REQUIRE(objectValue.GetType() == "Object");
}

TEST_CASE("ValueOperatorEqual", "[ValueTest]") {
    Value stringValue{};
    stringValue.FromString("Thirteen");

    Value numberValue{};
    numberValue.FromNumber(13);

    Object object{};
    object.AddValue(stringValue, numberValue);
    object.AddValue(numberValue, stringValue);

    Value valueFirst{};
    valueFirst.FromObject(object);

    Value valueSecond{};
    valueSecond.FromObject(object);

    REQUIRE(valueFirst == valueSecond);
}

TEST_CASE("ValueOperatorBool", "[ValueTest]") {
    Value undefinedValue{};
    Value trueNumberValue{13.0};
    Value falseNumberValue{0.0};
    Value trueStringValue{"True"s};
    Value falseStringValue{""s};
    Value trueBooleanValue{true};
    Value falseBooleanValue{false};
    Value trueReferenceValue{reinterpret_cast<void*>(0xDEAFBEEF)};
    Value falseReferenceValue{nullptr};
    Value trueObjectValue{Object{}};

    REQUIRE(!undefinedValue);
    REQUIRE(trueNumberValue);
    REQUIRE(!falseNumberValue);
    REQUIRE(trueStringValue);
    REQUIRE(!falseStringValue);
    REQUIRE(trueBooleanValue);
    REQUIRE(!falseBooleanValue);
    REQUIRE(trueReferenceValue);
    REQUIRE(!falseReferenceValue);
    REQUIRE(trueObjectValue);
}
