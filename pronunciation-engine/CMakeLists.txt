cmake_minimum_required(VERSION 3.20)
project(PronunciationEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

option(PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME "Link the ONNX Runtime Mobile backend (not implemented yet)" OFF)
option(PRONUNCIATION_ENGINE_BUILD_TESTS "Build the Unit test suite" ON)
option(PRONUNCIATION_ENGINE_BUILD_HARNESS "Build the CLI harness" ON)

add_library(pronunciation_engine STATIC
  src/Content/PhonemeInventory.cpp
  src/Content/ContentStore.cpp
  src/ContentValidation/ContentValidation.cpp
  src/AudioQuality/AudioQuality.cpp
  src/Acoustic/MockAcousticModel.cpp
  src/Acoustic/OnnxAcousticModel.cpp
  src/CTCAlignment/CTCAlignment.cpp
  src/PhonemeScoring/PhonemeScoring.cpp
  src/Prosody/Prosody.cpp
  src/DecisionRules/ErrorHistory.cpp
  src/DecisionRules/DecisionRules.cpp
  src/Engine.cpp
  src/CApi.cpp
)

target_include_directories(pronunciation_engine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

if(PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME)
  target_compile_definitions(pronunciation_engine PRIVATE PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME)
endif()

target_compile_options(pronunciation_engine PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra>
)

if(PRONUNCIATION_ENGINE_BUILD_HARNESS)
  add_executable(pronunciation-harness Harness/main.cpp)
  target_link_libraries(pronunciation-harness PRIVATE pronunciation_engine)
  target_include_directories(pronunciation-harness PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
endif()

if(PRONUNCIATION_ENGINE_BUILD_TESTS)
  enable_testing()

  add_executable(pronunciation_engine_tests
    Tests/Unit/TestFramework.h
    Tests/Unit/TestMain.cpp
    Tests/Unit/AudioQualityTests.cpp
    Tests/Unit/CTCAlignmentTests.cpp
    Tests/Unit/PhonemeScoringTests.cpp
    Tests/Unit/ProsodyTests.cpp
    Tests/Unit/DecisionRulesTests.cpp
    Tests/Unit/ContentTests.cpp
    Tests/Unit/EngineTests.cpp
    Tests/Unit/CApiTests.cpp
  )
  target_include_directories(pronunciation_engine_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Unit
  )
  target_compile_definitions(pronunciation_engine_tests PRIVATE
    PRONUNCIATION_ENGINE_RESOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources"
  )
  target_link_libraries(pronunciation_engine_tests PRIVATE pronunciation_engine)

  add_test(NAME pronunciation_engine_tests COMMAND pronunciation_engine_tests)
endif()
