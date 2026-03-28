# Commits Ahead of Master (tests2 branch)

## 🔴🔴🔴 START 🔴🔴🔴
- [ ] d85a0b375 - Marker commit

## Code Organization
- [ ] 1dccc7c61 - refactor: Comprehensive codebase reorganization and conventions
- [ ] 58faa9d6a - refactor: Remove dead public methods and tighten visibility
- [ ] 828849e63 - refactor: Rename all source files, directories to PascalCase
- [ ] bf630c293 - feat: Add copyright headers to source files missing them
- [ ] dfa876224 - chore: Update all copyright years to 2026
- [ ] 2fa605bf3 - fix: Add missing Qt header includes
- [ ] 88cb542fd - fix: Add missing pragma once to RegisterTypes.h
- [ ] d1ab9b526 - refactor: Reorder includes to industry standard across entire codebase
- [ ] 35aedd3e1 - refactor: Use full paths from project root for all #include directives
- [ ] b31145d0b - refactor: reorganize all headers by relatedness instead of alphabetical order

## Standards & Documentation
- [ ] ea66ea4f3 - docs: Add Doxygen configuration and document all headers
- [ ] 906a4a834 - docs: Add inline implementation comments to all .cpp files
- [ ] 124b5e869 - chore: Remove TODO markers from App/Main.cpp
- [ ] 10cae80c8 - chore: fix markdownlint errors across all project markdown files
- [ ] 7b501b3b4 - chore: drop Qt 5 support, require Qt 6.2+

## Build System & Code Quality
- [ ] 258ec30ae - feat: Enable compilation database generation
- [ ] b80b5dd51 - refactor: Extract resources into separate CMake object library
- [ ] b8121fd2e - build: Add Windows metadata target properties for versioning
- [ ] 1b4436608 - feat: Add global Unity build configuration
- [ ] 708ab599a - build: Enable PCH reuse from wiredpanda_lib for test executables
- [ ] fdb19ea3d - refactor: Remove project headers from PCH, keep only stable external headers
- [ ] bdf0b3d09 - build: Fix PCH reuse condition for test executables
- [ ] 98af51022 - chore: Treat external dependencies as system headers to suppress clazy warnings
- [ ] ec702f096 - build: Remove unused FORMS variable from add_library call
- [ ] 4dc83390b - build: Exclude additional Qt library paths from coverage report
- [ ] d4397e1db - refactor: Consolidate and optimize CMake/CTest configuration
- [ ] 59240fc54 - feat: Add parallel test execution to CMakePresets
- [ ] 35a04a13b - chore: enable strict compiler warnings and fix all diagnostics

## Installation & Deployment
- [ ] 91b39565a - ci: Update Qt versions and emsdk across all workflows
- [ ] e417faea1 - ci(deps): bump actions/checkout from 5 to 6 and actions/upload-artifact from 5 to 6
- [ ] 944c2621a - ci: Remove ninja-build from apt install (pre-installed on ubuntu-latest)
- [ ] 6cdc3e99d - fix: Enable RPATH for custom Qt installations
- [ ] c372dfeea - ci: Replace linuxdeployqt with go-appimage in deploy workflow
- [ ] 4b8afe741 - fix: Fix AppImage icon on Wayland via self-registration and correct DirIcon
- [ ] 053b1c5f6 - feat: Add installation rules for desktop integration

## CI/CD Configuration
- [ ] 4ae113604 - ci: run ctest with -V (verbose) to capture test output in CI logs
- [ ] 5715ea5fb - ci: set QT_FORCE_STDERR_LOGGING=1 to fix missing test output on Windows
- [ ] eba0a261f - ci: add Linux-only no-unity/no-PCH build to catch missing includes
- [ ] 8b6593338 - ci: add mega unity build to catch symbol collisions

## Examples & Paths
- [ ] 490017c24 - fix: Improve lcov coverage filtering and error handling
- [ ] 14404cd9e - fix: use lcov 2.0 built-in dark mode for coverage reports
- [ ] a451d8cfb - fix: resolve Examples path relative to executable for AppImage/macOS

## Testing & MCP
- [ ] 6ee47f20e - feat: Add variable-size Mux/Demux with dynamic painting and UI editor fixes
- [ ] 39a7216c1 - fix: show individual port names for multi-port IC outputs like Display7
- [ ] 35b06466d - fix: Start port numbering at 1 to match chip schematic convention
- [ ] 6b229c079 - fix: Guard null m_elementeditor and fix stale action list in menu insertion
- [ ] 1cd61f41d - feat: Implement MCP server and Python client for programmatic circuit control
- [ ] d6a954a0c - fix: correct logic element behavior and harden ArduinoCodeGen
- [ ] ea682fac3 - feat: Add 133-class test suite with hierarchical organization

## 🔴🔴🔴 FIX 🔴🔴🔴
- [ ] a213d464d - Marker commit

## Serialization & Path Fixes
- [ ] 836cf7d1d - fix: Move TruthTable pixmap generation out of paint() to fix macOS Qt 6 crash
- [ ] 6113bc82b - fix: Simplify IC file path resolution for cross-platform compatibility
- [ ] 239273148 - fix: Save connection count in ChangeInput/OutputSizeCommand
- [ ] 122c382ed - refactor: Stream integrity checks with proper error detection

## Memory & Connection Fixes
- [ ] 324f2f946 - fix: Restore GlobalProperties::currentDir after autosave
- [ ] caf3d4c98 - fix: discard legacy IC skin data on load; throw on real OOB
- [ ] 8fa7d1d91 - fix: AudioBox path resolution, relative storage, and external file copying
- [ ] 3f63a7b1f - fix: delete connections on ports removed during morph and restore on undo
- [ ] f43ee9162 - fix: ID instability and memory leak in ChangeInput/OutputSizeCommand

## Performance & Circuit Handling
- [ ] a767c0647 - fix: light theme broken when OS is in dark mode
- [ ] 06c156977 - fix: hide value combo for InputButton in element editor
- [ ] 4777b346d - fix: clear wire highlight when morphing elements
- [ ] a2178584d - fix: add cycle detection for circular IC file references
- [ ] 7b36d3d97 - perf: use batched drawPoints for scene grid background instead of per-point drawing

## Examples & Bug Fixes
- [ ] 76cc2d390 - fix: Save As now saves instead of opening the target file
- [ ] c7a1c91a4 - fix: prevent viewport jumps when dragging elements
- [ ] ee5bf69ae - fix: guard Select All against null tab to prevent crash
- [ ] 357b2e1a3 - fix: reconnect scene shortcuts on tab switch
- [ ] 4cc77756f - fix: correct IC selection highlight mismatch and stale mouse grab after context menu
- [ ] 2534f4d3c - chore: update example flip-flop files with sensible switch defaults

## 🔴🔴🔴 REFACTORING 🔴🔴🔴
- [ ] 268b7072e - Marker commit

## Application & Code Quality
- [ ] 4a23d6705 - fix: Apply clazy code quality improvements
- [ ] 17906140a - fix: Replace raw ElementMapping pointer with unique_ptr in IC
- [ ] c3b120a6c - refactor: Cache feedback flag and pre-allocate settling buffer
- [ ] 8aef314f2 - refactor: Replace qApp macro override with static Application::instance() method
- [ ] a1054eac4 - feat: Replace ElementType::IC type checks with polymorphism

## Type System & Polymorphism
- [ ] 3a9cf6079 - feat: Namespace command utility functions
- [ ] 71acaa3ed - refactor: Replace Qt metatype element system with self-registering ElementMetadata
- [ ] 3c9bd77d9 - refactor: Replace VERSION() macro with typed version constants
- [ ] bf49a05a9 - feat: Settings typed accessors, remove magic string keys

## Settings & Constants
- [ ] 0f00275be - refactor: Clarify interactive vs non-interactive mode handling
- [ ] 4208c2ff7 - refactor: Move gridSize from GlobalProperties to Scene
- [ ] 6db705db3 - refactor: Thread SerializationContext through deserialization, remove GlobalProperties::currentDir
- [ ] b7524475d - refactor: Move interactiveMode from GlobalProperties to Application
- [ ] a89e02597 - refactor: Delete GlobalProperties class, migrate maxRecentFiles to RecentFiles

## Serialization & IDs
- [ ] ee437d63f - refactor: Symmetric save/load serialization
- [ ] 464ca3150 - refactor: Counter-based port IDs with backwards compatibility

## Architecture & Logic
- [ ] db7ec9307 - refactor: Direct port-to-logic linking
- [ ] 330a85430 - refactor: Remove m_successors from LogicElement
- [ ] 223221f3c - refactor: Remove vestigial GraphicElement::m_priority property
- [ ] 63859eef4 - refactor: Clean up LogicElement architecture (structural only)
- [ ] 635c2c753 - refactor: Move output change tracking into LogicElement
- [ ] 290dca6dc - refactor: Eliminate LogicElement hierarchy — simulate directly on GraphicElement

## Logic Element Architecture
- [ ] 410d2f5f2 - refactor: Move contextDir from global to per-Scene property
- [ ] 8be3598b2 - refactor: Centralize contextDir management in Workspace::setCurrentFile()
- [ ] b03ca6859 - refactor: Replace global ElementFactory ID registry with per-scene ID lifecycle

## ID & Context Refactoring
- [ ] 3d28f7bdc - refactor: Decompose ElementEditor into focused, single-responsibility components
- [ ] 418d6bc60 - refactor: Decompose BeWavedDolphin into focused single-responsibility modules
- [ ] 84d9b7f06 - refactor: Decompose MainWindow into focused single-responsibility modules

## Module Decomposition
- [ ] 195dc47af - refactor: Extract connectSceneAction + drainPortConnections helpers
- [ ] c9ffb60b9 - refactor: Extract ConnectionManager from Scene
- [ ] 01864a772 - refactor: Extract ClipboardManager from Scene
- [ ] 10d4ef3d3 - refactor: Extract PropertyShortcutHandler from Scene
- [ ] 6ad3ad7ed - refactor: Extract VisibilityManager from Scene

## Manager Extraction Refactoring
- [ ] 734501ab9 - refactor: Named version predicates via VersionInfo.h
- [ ] 9295dff95 - refactor: Extract Serialization::readPreamble
- [ ] e8ada3781 - refactor: Scene::deserializationContext
- [ ] 135ebb6ee - refactor: Split GraphicElement serialization into GraphicElementSerializer.cpp
- [ ] e52e9e61b - refactor: Decouple ElementInfo.h from ElementFactory.h

## 🔴🔴🔴 FEATURES 🔴🔴🔴
- [ ] 2065320bb - Marker commit

## Utilities & Updates
- [ ] a559f9e9a - feat: Auto-migration with versioned backup and connection topology fix
- [ ] c704dfde7 - feat: add automatic update notification via GitHub Releases API

## Code Generation
- [ ] 7f7b52b3b - feat: overhaul Arduino codegen with IC support, board selection, and testbench validation
- [ ] 12f16fef5 - feat: Add SystemVerilog code generation with hierarchical IC support

## UI & Logic System
- [ ] e80e246028 - feat: surface feedback convergence warning to user via status bar
- [ ] a1d2e3471 - feat: implement 4-state logic system (Unknown/Inactive/Active/Error)
- [ ] 9b1b41b1c - feat: add System theme option to follow OS color scheme

## Wireless & Sentry
- [ ] b146e2d46 - feat: improve Sentry integration — user ID, breadcrumbs, tags, dedup
- [ ] 88ee8aca5 - feat: Add wireless Node Tx/Rx mode with full test coverage

## V4.6 Format & IC System
- [ ] c2247dd8c - feat: add ICDefinition/ICRegistry architecture with port metadata
- [ ] a1159fa31 - feat: embedded IC system with full test suite (147 tests)
- [ ] 5b1fe9982 - feat: unify file metadata into single QMap (V4.6 format)

## Recent Type & API Fixes
- [ ] 7dd0d4baa - refactor: remove misleading default parameters from 5 APIs
- [ ] f726561a1 - refactor: fix type mismatches in frequency/delay and simOutputSize APIs

## Documentation & Translations
- [ ] c472b9d3b - docs: improve Doxygen coverage across 17 header files
- [ ] 6a2b880ce - update translations
- [ ] 376d9db62 - Create COMMITS_AHEAD_OF_MASTER.md

---

**Total Commits:** 130
**Branch:** tests2 (ahead of master)

- the phase delay on clocks have the labels over the slider
- in the leftbar GND is not translated but selecting a GND element shows as TERRA, its inconsistent
- when i have a display7 as a output of an IC it should reflect each display7 portname in the IC ports but it doesnt and all of them just say display7
- make failures to load custom skins/audios not block loading the .panda file?
- remove default parameters in headers to avoid bugs (function had 3 parameters, changed to 4 and now the call sites are wrong)
- some tests catch exceptions but then dont use the exception message
- replace all delete/qscopedpointer with unique_ptr
- reformat code so it always has an empty line before/after {}
- replace new/delete with smart pointers?
- why coverage.yml uses gcov but coverage.sh uses lcov?
- coverage.sh should install all deps first
- add a doxygen step in the GH workflows to deploy to the site?
- should we add the arduino/verilog exported files to the repo's test folder for tracking regressions?
- profile
- why is IC.h/.cpp not in Element/GraphicElements/?
- why are some folders in singular and others in plural?
- BewavedDolphin_Ui => BewavedDolphinUI
- Led.cpp has duplicated skin paths (also some paths have double //)
- some files use \brief and others use @brief
- rename IC::icFile() to IC::file()?
- move the defaults ctor/dtor of ItemWithId to the header
- remove all Q_ASSERT
- remove [[no_discard]]
