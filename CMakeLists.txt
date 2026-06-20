# hydraforge-pdk — independent repo CMakeLists
# 功能描述: PDK 独立仓库根 CMakeLists (K3 决策, ADR-0021 §2.2)
#          此文件由 scripts/sync-pdk.sh 自动生成, 勿手动编辑
# 设计依据: ADR-0021 + openspec/changes/2026-07-07-pdk-skeleton
# 作者: AgenticDSL Phase 1 Sprint 4 (T4b)

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

# nlohmann_json 依赖 (bundled, 独立仓库版本)
target_include_directories(hydraforge_pdk INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann_json/single_include
)

# --- 测试 (独立仓库验证) ---
option(HYDRAFORGE_PDK_BUILD_TESTS "Build PDK standalone tests" ON)
if(HYDRAFORGE_PDK_BUILD_TESTS)
  enable_testing()

  # Standalone 测试 (不需要 Runtime, 仅测试 PDK 自身契约)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_pdk_macros_standalone.cpp")
    add_executable(test_pdk_macros
      tests/test_pdk_macros_standalone.cpp
      tests/catch_amalgamated.cpp
    )
    target_link_libraries(test_pdk_macros PRIVATE hydraforge_pdk)
    target_include_directories(test_pdk_macros PRIVATE tests)
    target_compile_definitions(test_pdk_macros PRIVATE
      CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS=1
      HYDRAFORGE_PDK_STANDALONE=1
    )
    add_test(NAME test_pdk_macros COMMAND test_pdk_macros)
    message(STATUS "[hydraforge_pdk] test_pdk_macros registered")
  endif()
endif()

# --- 安装规则 (Phase 2 后续) ---
include(GNUInstallDirs)
install(TARGETS hydraforge_pdk EXPORT hydraforge_pdkTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
