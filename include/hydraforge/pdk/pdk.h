// hydraforge-pdk — independent repo version
// PDK 统一入口 — 引用 3 个 PDK 子头 (standalone 仓库路径, 与 monorepo 一致)
// 注: 独立仓库中, tool_macros.h / agent_macros.h / safe_exec.h 直接位于此目录
// (无需 monorepo agenticdsl/pdk/ forward)

#pragma once

#include <hydraforge/pdk/tool_macros.h>
#include <hydraforge/pdk/agent_macros.h>
#include <hydraforge/pdk/safe_exec.h>

// PDK 版本 (供 Runtime/Plugin 编译时识别)
#define HYDRAFORGE_PDK_VERSION_MAJOR 0
#define HYDRAFORGE_PDK_VERSION_MINOR 1
#define HYDRAFORGE_PDK_VERSION_PATCH 0
#define HYDRAFORGE_PDK_VERSION "0.1.0"
