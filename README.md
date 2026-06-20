# hydraforge-pdk

**Plugin Development Kit (PDK) for HydraForge** — independent, header-only library for building domain plugins.

> **Status**: 🟡 Partial (v0.1.0, 2026-06-19, Sprint 4 MVP)
> This standalone repo is **auto-synced** from [HydraForge monorepo](https://github.com/chisuhua/HydraForge) via `scripts/sync-pdk.sh` (ADR-0021 §7 Dual-Repo Policy).
> Design: [ADR-0021 PDK Design](https://github.com/chisuhua/HydraForge/blob/main/docs/adr/adr-0021-pdk-design.md)

## Dual-Repo Policy

PDK is developed in the HydraForge monorepo (`include/agenticdsl/pdk/` + `pdk/`) and
periodically synced to this standalone repo for external consumers.

- **Vendored in monorepo** for zero-friction internal dev + tests
- **Published standalone** for downstream consumers via `find_package(hydraforge_pdk)`

See ADR-0021 §7 in HydraForge monorepo for full rationale.

## Overview

PDK provides a **standardized development toolkit** for HydraForge domain plugin authors, reducing boilerplate from ~20 lines to ~5 lines per tool:

| Component | Purpose | Status |
|-----------|---------|--------|
| `DECLARE_TOOL` macro | Tool registration scaffold (Schema + permissions + error handling) | ✅ MVP |
| `DEFINE_AGENT` macro | Agent loop template (React MVP; PlanExecute/ForkJoin TODO) | ✅ MVP (React) |
| `SafeExec` wrapper | Sandbox execution (timeout + exception; no fork/seccomp) | ✅ MVP |
| `ToolSpec` / `ToolParam` / `ToolPermissions` | Tool metadata structures | ✅ MVP |
| `FakeStateStore` / `StubLLM` / `MockSandbox` | Test doubles | 🔜 Phase 2 |
| `PluginLifecycle` | Plugin init/load/unload/health_check | 🔜 Phase 3 |
| Full SafeExec with fork/cgroups/seccomp | Process-level isolation | 🔜 Phase 3 |
| CMake project generator | `cmake_init()` / `project_template()` | 🔜 Phase 4 |

## Quick Start

```cpp
// my_plugin.cpp
#include <hydraforge/pdk/pdk.h>
#include <nlohmann/json.hpp>

using namespace hydraforge::pdk;

// DECLARE_TOOL: 5 lines of domain logic
DECLARE_TOOL(echo_tool, "Echo back input",
  return __pdk_args;
)

// DEFINE_AGENT: React loop MVP (auto-generated agent class)
DEFINE_AGENT(my_agent, AgentLoopType::React);
// Expands to: class my_agentAgent { ... };

// SafeExec: timeout + exception isolation
SafeExec().with_timeout(std::chrono::milliseconds(100))
          .run([] { /* your code */ });
```

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-13
cmake --build .
ctest --output-on-failure
```

**Requirements**: C++20, nlohmann_json (bundled), HydraForge Runtime (for full integration in Phase 2+).

## Design Principles (ADR-0021 P1-P6)

1. **P1**: PDK is **not** part of Runtime — independent repo (this one)
2. **P2**: PDK is **optional** — advanced developers can hand-write
3. **P3**: PDK **statically links** to plugins — Runtime has zero awareness, zero overhead
4. **P4**: PDK wraps only **general development patterns** — no domain logic
5. **P5**: PDK version is **decoupled** from Runtime — independent upgrade path
6. **P6**: PDK provides **test doubles** — plugins can be tested in isolation

## Repository Structure

```
hydraforge-pdk/
├── CMakeLists.txt                 # INTERFACE library (header-only)
├── README.md                      # This file
├── include/
│   └── hydraforge/
│       └── pdk/
│           ├── pdk.h              # Unified entry
│           ├── tool_macros.h      # DECLARE_TOOL
│           ├── agent_macros.h     # DEFINE_AGENT
│           └── safe_exec.h        # SafeExec
├── external/
│   └── nlohmann_json/             # Bundled header
└── tests/
    ├── catch_amalgamated.{hpp,cpp}  # Bundled Catch2
    └── test_pdk_macros_standalone.cpp
```

## Migration from Monorepo

The monorepo `HydraForge` (https://github.com/chisuhua/HydraForge) also vendors PDK at `include/agenticdsl/pdk/` and `pdk/`. The two paths are **API-compatible** — switching to standalone is a CMake change only:

**Monorepo** (vendored PDK):
```cmake
add_subdirectory(pdk)
target_link_libraries(my_plugin PRIVATE hydraforge_pdk)
```

**Standalone** (this repo):
```cmake
find_package(hydraforge_pdk 0.1 REQUIRED)
target_link_libraries(my_plugin PRIVATE hydraforge::pdk)
```

## Test Status

- **4/4** standalone test cases pass (31 assertions)
- Covers: DECLARE_TOOL expansion, SafeExec timeout, SafeExec exception, Runtime decoupling
- CI integration: pending (Phase 2)

## Related ADRs (in HydraForge monorepo)

- [ADR-0021 PDK Design](https://github.com/chisuhua/HydraForge/blob/main/docs/adr/adr-0021-pdk-design.md)
- [ADR-0022 Plugin Loading](https://github.com/chisuhua/HydraForge/blob/main/docs/adr/adr-0022-plugin-loading.md)
- [ADR-0019 IInteractionBus](https://github.com/chisuhua/HydraForge/blob/main/docs/adr/adr-0019-iinteraction-bus-mvp.md)
- [ADR-0020 Thread Model Isolation](https://github.com/chisuhua/HydraForge/blob/main/docs/adr/adr-0020-thread-model-isolation.md)

## License

TBD (per HydraForge project license)
