// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include "Blackboard/Object.h"
#include "Blackboard/Value.h"

#include <catch.hpp>

using Object = blackboard::Object;
using Value = blackboard::Value;

using namespace std::string_literals;

TEST_CASE("AddValue", "[ObjectTest]") {
    Value stringValue{"Thirteen"s};
    Value numberValue{13.0};

    Object object{};
    object.AddValue(stringValue, numberValue);
    object.AddValue(numberValue, stringValue);

    REQUIRE(object.GetValue(stringValue));
    REQUIRE(object.GetValue(stringValue)->ToNumber() == 13);

    REQUIRE(object.GetValue(numberValue));
    REQUIRE(object.GetValue(numberValue)->ToString() == "Thirteen");
}

TEST_CASE("GetValue", "[ObjectTest]") {
    Value stringValue{"Thirteen"s};
    Value numberValue{13.0};

    Object object{};
    object.AddValue(stringValue, numberValue);
    object.AddValue(numberValue, stringValue);

    REQUIRE(object.GetValue(stringValue));
    REQUIRE(object.GetValue(stringValue)->ToNumber() == 13);

    REQUIRE(object.GetValue(numberValue));
    REQUIRE(object.GetValue(numberValue)->ToString() == "Thirteen");
}

TEST_CASE("RemoveValue", "[ObjectTest]") {
    Value stringValue{"Thirteen"s};
    Value numberValue{13.0};

    Object object{};
    object.AddValue(stringValue, numberValue);
    object.AddValue(numberValue, stringValue);

    REQUIRE(object.GetValue(stringValue));
    REQUIRE(object.GetValue(stringValue)->ToNumber() == 13);

    REQUIRE(object.GetValue(numberValue));
    REQUIRE(object.GetValue(numberValue)->ToString() == "Thirteen");

    object.RemoveValue(numberValue);

    REQUIRE(!object.GetValue(numberValue));

    REQUIRE(object.GetValue(stringValue));
    REQUIRE(object.GetValue(stringValue)->ToNumber() == 13);
}

TEST_CASE("ObjectOperatorEqual", "[ObjectTest]") {
    Value stringValue{"Thirteen"s};
    Value numberValue{13.0};

    Object objectFirst{};
    objectFirst.AddValue(stringValue, numberValue);
    objectFirst.AddValue(numberValue, stringValue);

    Object objectSecond{};
    objectSecond.AddValue(stringValue, numberValue);
    objectSecond.AddValue(numberValue, stringValue);

    REQUIRE(objectFirst == objectSecond);
}
