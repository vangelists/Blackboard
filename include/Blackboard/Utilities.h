// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#pragma once

#include <variant>

namespace blackboard {

// Derived from https://stackoverflow.com/a/52303671.
//
template<typename VariantType, typename T, std::size_t index = 0>
constexpr std::size_t variant_index() {
    if constexpr (index == std::variant_size_v<VariantType>) {
        return index;
    } else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
        return index;
    } else {
        return variant_index<VariantType, T, index + 1>();
    }
}

} // namespace blackboard
