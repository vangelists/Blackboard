# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif ()

add_executable(BlackboardTest lib/Catch.cpp
                              BlackboardRegistryTest.cpp
                              BlackboardTest.cpp
                              ObjectTest.cpp
                              ValueTest.cpp
                              IntegrationTest.cpp)
set_target_properties(BlackboardTest PROPERTIES
    CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

target_include_directories(BlackboardTest PRIVATE "lib/Catch2-v2.11.1")
target_link_libraries(BlackboardTest Blackboard)

enable_testing()
add_test(BlackboardTest ${CMAKE_CURRENT_BINARY_DIR}/bin/BlackboardTest)
