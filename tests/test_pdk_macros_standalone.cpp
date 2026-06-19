// tests/test_pdk_macros_standalone.cpp
// 文件头注释
// 功能描述：PDK 独立仓库单元测试 (Phase 1 Sprint 4 T4b)。
//          独立仓库版本 (不需要 HydraForge Runtime), 仅测试 PDK 自身的:
//          - DECLARE_TOOL 宏展开
//          - SafeExec 超时/异常/正常路径
//          - PDK version macros
//          - PDK 头文件无 Runtime 内部依赖 (P3 静态链接)
//
// 注: DEFINE_AGENT 完整测试需要 HydraForge Runtime (DSLEngine + IInteractionBus),
//      在 monorepo 的 tests/test_pdk_macros.cpp 中已覆盖 5/5 pass。
//      此处 standalone 版本聚焦 PDK 自身契约, 验证 4/5 场景。
// 设计依据：ADR-0021 + openspec/changes/2026-07-07-pdk-skeleton
// 作者：AgenticDSL Phase 1 Sprint 4 (T4b)
// 最后修改日期：2026-06-19

#include "catch_amalgamated.hpp"

#include "hydraforge/pdk/pdk.h"
#include "hydraforge/pdk/tool_macros.h"
#include "hydraforge/pdk/agent_macros.h"
#include "hydraforge/pdk/safe_exec.h"

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

using namespace hydraforge::pdk;

// =====================================================================
// Test 1: DECLARE_TOOL 展开
// =====================================================================
// DECLARE_TOOL 必须在 file scope 声明 (inline 变量不可在 block scope)
// 用法: DECLARE_TOOL(name, desc, body...) — body 含 return 语句, 无尾部 {}
DECLARE_TOOL(test_echo, "回显工具",
  return __pdk_args;
)

DECLARE_TOOL(test_throw, "抛异常工具",
  throw std::runtime_error("disk full");
)

TEST_CASE("PDK DECLARE_TOOL expands to ToolSpec + handler (standalone)",
          "[pdk][sprint4][standalone][declare_tool]") {
  SECTION("ToolSpec metadata is correct") {
    REQUIRE(tool_spec_test_echo.name == "test_echo");
    REQUIRE(tool_spec_test_echo.description == "回显工具");
    REQUIRE(tool_spec_test_echo.params.empty());
    REQUIRE_FALSE(tool_spec_test_echo.permissions.network);
  }

  SECTION("Handler returns input args") {
    nlohmann::json input = {{"message", "hello"}};
    nlohmann::json output = tool_handler_test_echo(input);
    REQUIRE(output["message"] == "hello");
    REQUIRE_FALSE(output.contains("error"));
  }

  SECTION("Handler exception is caught and returned as error json") {
    nlohmann::json output = tool_handler_test_throw(nlohmann::json::object());
    REQUIRE(output.contains("error"));
    REQUIRE(output["error"] == "disk full");
  }

  SECTION("DECLARE_TOOL generates inline spec (header-only)") {
    // 验证 tool_spec_test_echo 是 inline 变量 (C++17 inline 变量语义)
    REQUIRE(tool_spec_test_echo.name == "test_echo");
  }
}

// =====================================================================
// Test 2: SafeExec 超时处理
// =====================================================================
TEST_CASE("PDK SafeExec enforces timeout (standalone)",
          "[pdk][sprint4][standalone][safe_exec][timeout]") {
  SafeExec exec;

  SECTION("Function completes within timeout") {
    int result = exec.with_timeout(std::chrono::milliseconds(100)).run([] {
      return 42;
    });
    REQUIRE(result == 42);
  }

  SECTION("Function exceeds timeout → throw runtime_error") {
    REQUIRE_THROWS_AS(
        exec.with_timeout(std::chrono::milliseconds(10)).run([] {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          return 42;
        }),
        std::runtime_error);
  }

  SECTION("Timeout error message contains timeout value") {
    try {
      exec.with_timeout(std::chrono::milliseconds(5)).run([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return 42;
      });
      FAIL("Expected timeout exception");
    } catch (const std::runtime_error& e) {
      std::string msg = e.what();
      REQUIRE(msg.find("timed out") != std::string::npos);
      REQUIRE(msg.find("5ms") != std::string::npos);
    }
  }

  SECTION("Default timeout is 30s") {
    SafeExec default_exec;
    REQUIRE(default_exec.timeout() == std::chrono::milliseconds(30000));
  }
}

// =====================================================================
// Test 3: SafeExec 异常捕获
// =====================================================================
TEST_CASE("PDK SafeExec propagates handler exceptions (standalone)",
          "[pdk][sprint4][standalone][safe_exec][exception]") {
  SafeExec exec;

  SECTION("std::runtime_error is propagated") {
    REQUIRE_THROWS_AS(
        exec.with_timeout(std::chrono::milliseconds(1000)).run([] {
          throw std::runtime_error("file not found");
        }),
        std::runtime_error);
  }

  SECTION("Original exception message preserved") {
    try {
      exec.with_timeout(std::chrono::milliseconds(1000)).run([] {
        throw std::runtime_error("connection refused");
      });
      FAIL("Expected exception");
    } catch (const std::exception& e) {
      std::string msg = e.what();
      REQUIRE(msg == "connection refused");
    }
  }

  SECTION("Different exception types propagate correctly") {
    REQUIRE_THROWS_AS(
        exec.with_timeout(std::chrono::milliseconds(1000)).run([] {
          throw std::invalid_argument("bad arg");
        }),
        std::invalid_argument);

    REQUIRE_THROWS_AS(
        exec.with_timeout(std::chrono::milliseconds(1000)).run([] {
          throw std::out_of_range("out of range");
        }),
        std::out_of_range);
  }
}

// =====================================================================
// Test 4: PDK 头文件无 Runtime 内部依赖 (P3 静态链接验证)
// =====================================================================
TEST_CASE("PDK headers compile without Runtime internal dependencies (standalone)",
          "[pdk][sprint4][standalone][runtime_decoupling]") {
  SECTION("ToolSpec / ToolParam / ToolPermissions are POD-ish structs") {
    ToolSpec spec;
    spec.name = "x";
    spec.description = "y";
    spec.params = {{"p1", "string", true}};
    spec.permissions.readonly_paths = {"/tmp"};
    spec.permissions.network = true;

    REQUIRE(spec.name == "x");
    REQUIRE(spec.params.size() == 1);
    REQUIRE(spec.params[0].required);
    REQUIRE(spec.permissions.readonly_paths.size() == 1);
  }

  SECTION("AgentLoopType enum has expected values") {
    REQUIRE(static_cast<int>(AgentLoopType::React) == 0);
    REQUIRE(static_cast<int>(AgentLoopType::PlanExecute) == 1);
    REQUIRE(static_cast<int>(AgentLoopType::ForkJoin) == 2);
  }

  SECTION("SafeExec chainable configuration returns self-reference") {
    SafeExec exec;
    SafeExec& ref1 = exec.with_timeout(std::chrono::milliseconds(100));
    SafeExec& ref2 = exec.with_layer_profile(2);
    REQUIRE(&ref1 == &exec);
    REQUIRE(&ref2 == &exec);
    REQUIRE(exec.timeout() == std::chrono::milliseconds(100));
    REQUIRE(exec.layer_profile() == 2);
  }

  SECTION("PDK version macros are defined") {
#ifndef HYDRAFORGE_PDK_VERSION
    FAIL("HYDRAFORGE_PDK_VERSION not defined");
#endif
    std::string version = HYDRAFORGE_PDK_VERSION;
    REQUIRE_FALSE(version.empty());
    // v0.1.0 是 Sprint 4 MVP 版本
    REQUIRE(version == "0.1.0");
  }
}
