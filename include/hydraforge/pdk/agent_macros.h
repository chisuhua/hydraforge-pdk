// include/hydraforge/pdk/agent_macros.h (standalone version)
// 文件头注释
// 功能描述: DEFINE_AGENT 宏 — Agent 循环脚手架 (ADR-0021 §3.2, 独立仓库版本)
//          展开为 class XXXAgent 含 run(prompt) 方法。MVP 仅支持 React 循环。
// 设计依据: ADR-0021 + openspec/changes/2026-07-07-pdk-skeleton
// 注: 此文件由 scripts/sync-pdk.sh 自动生成 (从 monorepo 版本转换),
//     勿手动编辑。monorepo 原版在 include/agenticdsl/pdk/agent_macros.h

#pragma once

// Standalone 版本: 使用前向声明 + minimal stubs (无 monorepo Runtime 依赖)
// 真实使用需链接 HydraForge Runtime (find_package(hydraforge))

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <utility>

namespace agenticdsl {

// 前向声明 Runtime 类型 (Phase 2 链接时由 HydraForge Runtime 提供完整定义)
class DSLEngine;
class SimpleCognitiveOrchestrator;
class IInteractionBus;

// Minimal ToolResult stub (用于 standalone MVP, Phase 2 替换为 Runtime 版本)
struct ToolResult {
  bool ok = false;
  nlohmann::json data = nlohmann::json::object();
  nlohmann::json meta = nlohmann::json::object();
};

} // namespace agenticdsl

namespace hydraforge::pdk {

enum class AgentLoopType {
  React,
  PlanExecute,
  ForkJoin,
};

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
      agenticdsl::ToolResult result;                                                   \
      result.ok = true;                                                                \
      result.meta["prompt"] = prompt;                                                  \
      result.meta["note"] = "DEFINE_AGENT standalone MVP: orch not invoked";            \
      return result;                                                                  \
    }                                                                                  \
   private:                                                                            \
    std::unique_ptr<agenticdsl::DSLEngine> engine_;                                   \
    std::shared_ptr<agenticdsl::IInteractionBus> bus_;                                 \
  };

} // namespace hydraforge::pdk
