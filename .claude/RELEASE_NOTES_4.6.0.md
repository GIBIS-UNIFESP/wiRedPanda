## 🎯 For Users

### 🚀 New Features

- **4-state logic system**: Signals now carry four distinct states — Unknown, Inactive, Active, and Error — for more accurate and informative simulation feedback
- **Embedded IC system**: ICs can now optionally be stored directly inside `.panda` files, making circuits self-contained and easier to share — file-based ICs continue to work as before
- **SystemVerilog code generation**: Export circuits to SystemVerilog with hierarchical IC support
- **Enhanced Arduino codegen**: Significantly improved code generation with IC support, board selection, and testbench validation
- **Variable-size Mux/Demux**: Flexible multiplexer and demultiplexer elements with dynamic input/output sizing
- **Wireless Node Tx/Rx mode**: Nodes can now operate in wireless transmitter/receiver mode for cleaner circuit layouts
- **Automatic update notifications**: Application now checks for new releases from GitHub and notifies users
- **Auto-migration**: Automatic file format migration with versioned backup
- **System theme**: New "System" theme option that follows the OS color scheme
- **Feedback convergence warning**: Simulation now surfaces feedback loop convergence warnings to the status bar
- **Labeled slider for clock control**: New UI control for precise clock phase delay adjustment

### ⚡ Performance

- **Grid rendering**: Replaced per-point drawing with tiled pixmap for significantly faster scene rendering

### 🐛 Bug Fixes

**Crashes & data loss**

- Fixed crashes and incorrect behavior when working with multiple tabs simultaneously
- Fixed IC loading failures caused by autosave corrupting file paths in other open tabs
- Fixed macOS crash when using Truth Table elements (Qt 6)
- Fixed crash on Select All with no open tab
- Fixed null pointer safety across core components
- Fixed null element editor and stale action list in menu insertion

**IC components**

- Fixed IC loading failure when opening files created on a different operating system
- Fixed Windows-specific failure when migrating IC files
- Fixed legacy skin data causing out-of-bounds errors on load
- Fixed cycle detection for circular IC file references
- Fixed IC selection highlight mismatch and stale mouse grab after context menu
- Fixed individual port names not showing for multi-port IC outputs like Display7
- Fixed port numbering to start at 1 to match chip schematic convention

**Undo/redo**

- Fixed instability when splitting connections
- Fixed morph operations dropping or leaking connections on undo
- Fixed ID instability and memory leak when changing element input/output count

**Simulation**

- Fixed inconsistent behavior between runs
- Fixed incorrect clock timing when resuming from pause
- Fixed incorrect logic element behavior and hardened Arduino code generation

**UI & editor**

- Fixed Save As opening the target file instead of saving
- Fixed viewport jumps when dragging elements
- Fixed light theme broken when OS is in dark mode
- Fixed wire highlight not clearing when morphing elements
- Fixed value combo showing for InputButton in element editor
- Fixed element icons not updating when switching application theme
- Fixed element search tab filtering using wrong tab index
- Fixed scene shortcuts disconnecting on tab switch

**Other fixes**

- Fixed AudioBox path resolution, relative storage, and external file copying
- Fixed incorrect resource paths for AudioBox/Buzzer elements
- Fixed Examples path resolution for AppImage/macOS bundles
- Restored Windows executable metadata with proper resource file

---

## 🔧 For Developers

### 🏗️ Architecture

- **4-state logic engine**: Complete simulation overhaul from boolean to four-state (Unknown/Inactive/Active/Error) signal model
- **Embedded IC architecture**: New ICDefinition/ICRegistry system with port metadata, supporting both embedded and file-based IC workflows
- **Unified file metadata**: Single QMap-based metadata store (V4.6 format)
- **Per-scene element ID management**: Complete ElementFactory refactoring, removed global ID registry
- **Global state elimination**: Deleted GlobalProperties class; migrated members to Application, Scene, and RecentFiles; eliminated mutable static state from IC by threading SerializationContext through the deserialization chain
- **LogicElement simplification**: Eliminated LogicElement hierarchy — simulation runs directly on GraphicElement; removed vestigial m_priority and m_successors; cached feedback flag with pre-allocated settling buffer
- **Polymorphic IC interface**: Eliminated manual IC type checking with polymorphic dispatch
- **Self-registering ElementMetadata**: Replaced Qt metatype system with compile-time validated metadata declarations
- **Direct port-to-logic linking**: Optimized connection architecture without intermediate successors
- **Simulation priority extraction**: Priority calculation extracted into Priorities.h with Tarjan's SCC feedback detection
- **Counter-based port IDs**: New port ID scheme with backwards compatibility for older file formats
- **Memory safety**: Replaced raw ElementMapping pointer with unique_ptr in IC; comprehensive null pointer safety pass
- **Headless mode detection**: Comprehensive display-less environment support for CI and server contexts

### 🔄 Refactoring

- **Scene decomposition**: Extracted VisibilityManager, PropertyShortcutHandler, ClipboardManager, ConnectionManager, connectSceneAction, and drainPortConnections helpers from Scene
- **Serialization decomposition**: Split GraphicElement serialization into GraphicElementSerializer; extracted readPreamble; added named version predicates via VersionInfo.h; decoupled ElementInfo.h from ElementFactory.h; added Scene::deserializationContext
- **Module decomposition**: MainWindow, BeWavedDolphin, and ElementEditor refactored into single-responsibility components
- **Settings modernization**: Replaced magic string keys with typed accessors
- **Symmetric serialization**: Save and load paths now use matching serialization logic
- **Tab management**: Replaced fragile tab indices with object name-based lookups in MainWindow
- **Namespace organization**: Command utilities now properly namespaced
- **Safe casting**: Replaced qgraphicsitem_cast with appropriate alternatives for subclass casting
- **Stream integrity checks**: Proper error detection for corrupted data streams
- **Context directory management**: Centralized contextDir in Workspace::setCurrentFile(); moved from global to per-Scene property
- **Application singleton**: Replaced qApp macro override with static Application::instance() method
- **Version constants**: Replaced VERSION() macro with typed version constants
- **Mode handling**: Clarified interactive vs non-interactive mode handling
- **Header organization**: Reorganized all headers by relatedness instead of alphabetical order
- **Dead code removal**: Removed unused serialization artifacts from QNEPort; removed obsolete Qt version checks below 5.15
- **PCH optimization**: Removed project headers from PCH, keeping only stable external headers
- **Build configuration**: Consolidated and optimized CMake/CTest configuration; modernized CMake syntax across all workflows
- **Testing infrastructure**: Modernized with CTest integration; added test timeouts
- **API cleanup**: Fixed type mismatches in frequency/delay and simOutputSize APIs; removed misleading default parameters from 5 APIs

### 📦 Build System

- **Qt 6.2+ required**: Dropped Qt 5 support entirely
- **CMake presets**: Comprehensive configuration for all build variants (debug, release, coverage, sanitizers)
- **Link-time optimization (LTO)**: Enabled for release builds
- **Global Unity build + PCH reuse**: Parallel compilation acceleration for app and test targets
- **Desktop integration (Linux)**: CMake installation rules for FreeDesktop environment
- **Windows metadata**: Professional versioning and branding information in executables
- **Compilation database generation**: Full support for IDE integration and analysis tools
- **RPATH support**: Custom Qt installation compatibility
- **Resource library**: Extracted resources into separate CMake object library
- **Standardized output**: Consistent binary output location across all generators
- **Strict compiler warnings**: Enabled strict warnings and fixed all diagnostics
- **System headers for dependencies**: External dependencies treated as system headers to suppress clazy warnings
- **Build fixes**: Fixed PCH reuse condition for test executables; removed unused FORMS variable; excluded additional Qt library paths from coverage report

### 🧪 Testing

- **137-class comprehensive test suite**: Hierarchical organization covering all major components including embedded ICs
- **Parallel test execution**: Automatic CTest multi-threaded execution for faster feedback
- **Backward compatibility regression suite**: Extensive .panda file format compatibility testing
- **Coverage collection**: Improved lcov filtering and error handling with dark theme integration; removed redundant gcov step; added Qt framework filtering

### 🌐 Deployment & CI

- **Qt 6.10.2 LTS upgrade**: Latest stable Qt version with improved performance
- **Multi-stage Docker builds**: Optimized DevContainer with Docker Hub integration
- **DevContainer Qt 6 upgrade**: Migrated from Qt 5.15 to Qt 6.10.2 with VS Code recommended extensions and Qt tooling
- **go-appimage deployment**: Replaced linuxdeployqt for improved AppImage generation
- **AppImage desktop integration**: Proper .desktop file and icon installation, Wayland icon fix
- **Windows 64-bit only**: Modernized Windows target to current standards with Ninja generator
- **Security-hardened workflows**: Granular GitHub Actions permissions configuration; replaced pbeast/gha-setup-vsdevenv with seanmiddleditch/gha-setup-vsdevenv@v5
- **Dependabot integration**: Automated GitHub Actions version management (actions/upload-artifact v4→v6, actions/checkout v5→v6, github/codeql-action v3→v4)
- **Sentry integration**: Comprehensive crash reporting with user ID, breadcrumbs, tags, and deduplication
- **CI improvements**: Actions v6 upgrades, Node.js 24 support, Linux no-unity/no-PCH build validation, verbose test output on Windows, mega unity build for symbol collision detection, consolidated build workflow steps
- **CI fixes**: Fixed aqtinstall version syntax; fixed Translation Management workflow caching error; fixed missing test output on Windows via QT_FORCE_STDERR_LOGGING; updated windows-qt preset to Visual Studio 2022 generator
- **Legacy cleanup**: Removed unused installer directory and legacy build/packaging scripts

### 📚 Code Quality & Documentation

- **PascalCase standardization**: All source files and directories renamed for consistency
- **Full path #include directives**: Industry-standard header organization
- **Specific Qt header includes**: Reduced compilation dependencies
- **Clazy improvements**: Eliminated idiomatic Qt misuses
- **Resource reorganization**: Restructured app/resources directory for better maintainability
- **Doxygen configuration**: Complete API documentation system setup
- **Inline and header documentation**: All .cpp and public API files documented with design intent
- **Copyright headers**: Added to all source files with 2026 update
- **Contributing guidelines**: Comprehensive documentation for external contributors
- **Markdown linting**: Fixed formatting errors across all project markdown files
- **Build documentation**: Completed build requirements in all README files
- **Project management**: Migrated task management from TODO.md to GitHub project

### 📁 File Format

- **v4.6.0 format**: Unified QMap-based metadata with embedded IC definitions
- **Auto-migration**: Seamless upgrade from older formats with versioned backup files
- **Example files**: Updated flip-flop examples with sensible switch defaults

### 🌍 Translations

- Updated translations for 39 languages: Synced new strings from 4-state logic, wireless node, embedded ICs, and auto-migration features

---

## 🙏 Contributors

Thank you to all contributors who helped improve wiRedPanda in this release:

- @darktorres — Primary development
- @GabrielGA-01 — SystemVerilog codegen development
- @gbonavina — Arduino codegen contributions
- @mams76 — Wireless Node Tx/Rx and Embedded IC contributions

---

**Full Changelog:** https://github.com/GIBIS-UNIFESP/wiRedPanda/compare/4.3.0...4.6.0
