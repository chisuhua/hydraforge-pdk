# hydraforge-pdk — independent repo CMakeLists
# 功能描述: PDK 独立仓库根 CMakeLists (K3 决策, ADR-0021 §2.2)
#          提供 hydraforge::pdk INTERFACE 库 + 测试编译
# 设计依据: ADR-0021 + openspec/changes/2026-07-07-pdk-skeleton
# 作者: AgenticDSL Phase 1 Sprint 4 (T4b)
# 最后修改日期: 2026-06-19

cmake_minimum_required(VERSION 3.20)
project(hydraforge_pdk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- PDK INTERFACE 库 (header-only) ---
add_library(hydraforge_pdk INTERFACE)
add_library(hydraforge::pdk ALIAS hydraforge_pdk)

target_include_directories(hydraforge_pdk INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(hydraforge_pdk INTERFACE cxx_std_20)

# nlohmann_json 依赖 (PDK 使用 ToolSpec + json 错误返回)
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
  target_link_libraries(hydraforge_pdk INTERFACE nlohmann_json::nlohmann_json)
else()
  # Fallback: assume nlohmann_json headers are available system-wide
  target_include_directories(hydraforge_pdk INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann_json/single_include
  )
  message(STATUS "[hydraforge_pdk] nlohmann_json not found via find_package; using bundled headers path")
endif()

# --- 测试 (独立仓库验证) ---
option(HYDRAFORGE_PDK_BUILD_TESTS "Build PDK standalone tests" ON)
if(HYDRAFORGE_PDK_BUILD_TESTS)
  enable_testing()

  # Catch2 (使用单头 amalgamated 版本, 与 HydraForge monorepo 一致)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/catch_amalgamated.hpp")
    message(STATUS "[hydraforge_pdk] Downloading Catch2 amalgamated header...")
    file(DOWNLOAD
      https://github.com/catchorg/Catch2/releases/download/v3.7.0/catch_amalgamated.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch_amalgamated.hpp
      SHOW_PROGRESS
    )
  endif()

  add_executable(test_pdk_macros
    tests/test_pdk_macros_standalone.cpp
    tests/catch_amalgamated.cpp
  )
  target_link_libraries(test_pdk_macros PRIVATE hydraforge_pdk)
  target_include_directories(test_pdk_macros PRIVATE tests)
  target_compile_definitions(test_pdk_macros PRIVATE
    CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1
    # standalone repo: PDK headers 位于 hydraforge/pdk/, 非 agenticdsl/pdk/
    HYDRAFORGE_PDK_STANDALONE=1
  )

  add_test(NAME test_pdk_macros COMMAND test_pdk_macros)
  message(STATUS "[hydraforge_pdk] test_pdk_macros registered (HYDRAFORGE_PDK_STANDALONE=1)")
endif()

# --- 安装规则 (Phase 2 后续) ---
include(GNUInstallDirs)
install(TARGETS hydraforge_pdk EXPORT hydraforge_pdkTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
