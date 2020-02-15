# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2020 Vangelis Tsiatsianas

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

add_library(Blackboard SHARED BlackboardRegistry.cpp
                              Blackboard.cpp
                              Object.cpp
                              Value.cpp)

set_target_properties(Blackboard PROPERTIES
    CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

target_include_directories(Blackboard PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_sources(Blackboard PUBLIC
    ${CMAKE_SOURCE_DIR}/include/Blackboard/Blackboard.h
    ${CMAKE_SOURCE_DIR}/include/Blackboard/BlackboardRegistry.h
    ${CMAKE_SOURCE_DIR}/include/Blackboard/Object.h
    ${CMAKE_SOURCE_DIR}/include/Blackboard/Utilities.h
    ${CMAKE_SOURCE_DIR}/include/Blackboard/Value.h
)
