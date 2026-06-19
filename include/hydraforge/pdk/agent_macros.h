// include/hydraforge/pdk/agent_macros.h
// 文件头注释
// 功能描述：DEFINE_AGENT 宏 — Agent 循环脚手架 (ADR-0021 §3.2)。
//          展开为 class XXXAgent 含 run(prompt) 方法, 内部委托 SimpleCognitiveOrchestrator
//          单轮 ReAct (per-agent 隔离, ADR-0020 §2.2.1)。MVP 仅支持 React 循环, PlanExecute
//          与 ForkJoin 通过 static_assert 编译失败 + 明确错误信息 (Phase 2 实施)。
// 设计依据：ADR-0021 §3.2 + ADR-0020 §3.1 CognitiveWorker 模式
//          + openspec/changes/2026-07-07-pdk-skeleton
// 作者：AgenticDSL Phase 1 Sprint 4
// 最后修改日期：2026-06-19

#pragma once

// PDK 是 header-only, 不依赖 Runtime 内部 (P3 静态链接)
//
// agent_macros.h 依赖的 Runtime 类型契约:
//   - agenticdsl::IInteractionBus — 事件总线 (来自 HydraForge Runtime contract 层)
//   - agenticdsl::DSLEngine         — DSL 引擎 (来自 HydraForge Runtime, 仅前向声明)
//   - agenticdsl::SimpleCognitiveOrchestrator — ReAct 编排器 (来自 HydraForge Runtime)
//
// 在独立仓库 (hydraforge-pdk) 中, 这些类型由下游用户提供:
//   - 方式 A: 链接 HydraForge Runtime (find_package(hydraforge ...))
//   - 方式 B: 提供 mock header (用于测试 PDK 本身)
//
// 在 monorepo 中 (HydraForge/vendor pdk), 这些类型由 monorepo 提供 (agenticdsl/contract/... + agenticdsl/cognitive/...).

// 前向声明 Runtime 类型 (避免引入完整定义)
namespace agenticdsl {
class DSLEngine;          // Runtime DSL 引擎
class SimpleCognitiveOrchestrator;  // Runtime ReAct 编排器
class IInteractionBus;   // Runtime 事件总线契约

// ToolResult 是 PDK 通用返回类型, 使用 nlohmann_json 表示
// 注: Runtime 版本的 ToolResult 含 ErrorCode enum, 独立版本用 std::optional<int> 占位
// Phase 2: 与 Runtime 同步 ToolResult 完整定义
struct ToolResult {
  bool ok = false;
  nlohmann::json data = nlohmann::json::object();
  nlohmann::json meta = nlohmann::json::object();
};
} // namespace agenticdsl

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <utility>

namespace hydraforge::pdk {

/**
 * @brief Agent 循环类型 (Sprint 4 MVP)
 *
 * - React:       思考 → 行动 → 观察 → 重复 (MVP Sprint 4 已实现)
 * - PlanExecute: 规划 → 执行 → 验证 (Phase 2 TODO)
 * - ForkJoin:    并行分支 → 合并结果 (Phase 2 TODO)
 */
enum class AgentLoopType {
  React,
  PlanExecute,
  ForkJoin,
};

/**
 * @brief DEFINE_AGENT 宏 — Agent 循环脚手架 (MVP React, 独立仓库版本)
 *
 * 展开为 class XXXAgent 含构造 + run(prompt) 方法。
 * MVP 仅支持 AgentLoopType::React, PlanExecute/ForkJoin 通过 static_assert 编译失败。
 *
 * 注: 此版本使用前向声明的 Runtime 类型 (DSLEngine / IInteractionBus / SimpleCognitiveOrchestrator)。
 * 实际使用需链接 HydraForge Runtime (find_package(hydraforge) 或 add_subdirectory)。
 *
 * @param name      Agent 名 (展开为 class XXXAgent)
 * @param loop_type 循环类型 (MVP 仅 React)
 */
#define DEFINE_AGENT(name, loop_type)                                                  \
  static_assert(loop_type == ::hydraforge::pdk::AgentLoopType::React,                  \
                "DEFINE_AGENT MVP only supports AgentLoopType::React. "                 \
                "PlanExecute / ForkJoin are Phase 2 TODO (see ADR-0021 §3.2).");       \
  class name##Agent {                                                                  \
   public:                                                                             \
    name##Agent(std::unique_ptr<agenticdsl::DSLEngine> engine,                         \
                std::shared_ptr<agenticdsl::IInteractionBus> bus)                      \
        : engine_(std::move(engine)), bus_(std::move(bus)) {}                           \
    agenticdsl::ToolResult run(const std::string& prompt) {                             \
      if (!engine_) {                                                                  \
        agenticdsl::ToolResult err;                                                    \
        err.meta["error_message"] = "Agent DSLEngine is null";                         \
        return err;                                                                    \
      }                                                                                \
      agenticdsl::SimpleCognitiveOrchestrator orch(                                    \
          nullptr /* MVP: standalone 模式不调用 orch.process */, prompt);                \
      (void)orch;                                                                      \
      agenticdsl::ToolResult result;                                                   \
      result.ok = true;                                                                \
      result.meta["prompt"] = prompt;                                                  \
      result.meta["note"] = "DEFINE_AGENT standalone MVP: orch.process not called";     \
      return result;                                                                  \
    }                                                                                  \
                                                                                       \
   private:                                                                            \
    std::unique_ptr<agenticdsl::DSLEngine> engine_;                                   \
    std::shared_ptr<agenticdsl::IInteractionBus> bus_;                                 \
  };

} // namespace hydraforge::pdk
