// include/hydraforge/pdk/pdk.h (standalone version)
// 文件头注释
// 功能描述: PDK 统一入口 (独立仓库版本, ADR-0021 §7)
//          引用 3 个子头 + 版本宏
// 设计依据: ADR-0021 + openspec/changes/2026-07-07-pdk-skeleton
// 注: 此文件由 scripts/sync-pdk.sh 自动生成, 勿手动编辑
//     (monorepo 路径在 include/agenticdsl/pdk/pdk.h)

#pragma once

#include <hydraforge/pdk/tool_macros.h>
#include <hydraforge/pdk/agent_macros.h>
#include <hydraforge/pdk/safe_exec.h>

// PDK 版本 (与 monorepo 同步, 由 sync-pdk.sh 注入)
#ifndef HYDRAFORGE_PDK_VERSION
#define HYDRAFORGE_PDK_VERSION_MAJOR 0
#define HYDRAFORGE_PDK_VERSION_MINOR 1
#define HYDRAFORGE_PDK_VERSION_PATCH 0
#define HYDRAFORGE_PDK_VERSION "0.1.0"
#endif
