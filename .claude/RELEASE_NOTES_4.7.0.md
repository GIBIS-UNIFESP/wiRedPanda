## 🎯 For Users

### 🚀 New Features

- **4-state logic**: Signals now carry four states — Unknown, Inactive, Active, and Error — for more accurate simulation feedback
- **Embedded ICs**: ICs can be stored directly inside `.panda` files, making circuits self-contained and easier to share
- **SystemVerilog export**: Export circuits to SystemVerilog with hierarchical IC support
- **Enhanced Arduino codegen**: Improved code generation with IC support, board selection, and testbench validation
- **Variable-size Mux/Demux**: Multiplexer and demultiplexer elements with dynamic input/output sizing
- **Wireless Node Tx/Rx**: Nodes can operate in wireless transmitter/receiver mode for cleaner layouts
- **Per-state appearance editing**: Multi-state elements support editing appearances for each state
- **Auto-update notifications**: Checks for new releases from GitHub and notifies users
- **Auto-migration**: Automatic file format migration with versioned backup
- **System theme**: Follows the OS color scheme
- **Buzzer improvements**: Arbitrary frequency (20–20,000 Hz) and volume control
- **Clock slider**: Labeled slider for precise clock phase delay adjustment

### ⚡ Performance

- Batched grid rendering for significantly faster scene drawing
- Cached audio device detection on Windows
- Inlined simulation hot paths and pre-filtered IC boundary elements
- Throttled visual updates to monitor refresh rate during simulation
- Lightweight labels, cached pixmaps, and deferred font loading

### 🐛 Bug Fixes

- Fixed crashes when working with multiple tabs simultaneously
- Fixed IC loading failures across different operating systems
- Fixed macOS crash with Truth Table elements on Qt 6
- Fixed undo/redo instability when splitting connections or morphing elements
- Fixed inconsistent simulation behavior between runs and incorrect clock timing on resume
- Fixed Save As opening the target file instead of saving
- Fixed viewport jumps when dragging elements
- Fixed numerous IC-related issues: cycle detection, port numbering, embedded blob corruption, port metadata ordering
- Fixed flip horizontal doing nothing due to inverted condition check
- Fixed flip horizontal and vertical producing identical results on asymmetric elements (e.g., D-latch) by using true mirror transforms instead of rotation
- Fixed element icons not updating on theme switch
- Fixed Examples path resolution for AppImage/macOS bundles
- Fixed WebAssembly issues: persistent settings, bundled fonts for non-Latin scripts, dark/light theme detection, and preloaded examples

## 🔧 For Developers

### 🏗️ Architecture

- Complete simulation overhaul from boolean to four-state signal model
- New ICRegistry system supporting both embedded and file-based IC workflows
- Per-scene element ID management; removed global ID registry
- Eliminated GlobalProperties class; migrated to Application, Scene, and RecentFiles
- Eliminated LogicElement hierarchy — simulation runs directly on GraphicElement
- Unified external file dependency management via `GraphicElement::externalFiles()`
- Self-registering ElementMetadata with compile-time validated declarations
- Tarjan's SCC-based feedback detection for simulation priority ordering
- Separated file format version from application version

### 📦 Build & Infrastructure

- **Qt 6.2+ required** — dropped Qt 5 support
- CMake presets for all build variants (debug, release, coverage, sanitizers)
- Unity build + PCH for faster compilation
- Migrated to Ubuntu 24.04 LTS DevContainer with Qt 6.11.0
- Sentry improvements: breadcrumbs, user ID, tags, and deduplication
- Security-hardened GitHub Actions workflows with Dependabot
- AppImage now bundles glibc and all dependencies, removing previous distro compatibility limitations
- Windows 64-bit only

### 🧪 Testing

- Expanded from 36 tests to 1,722 tests in a single unified binary
- GUI-level test suites for MainWindow, BeWavedDolphin, and FileDialogProvider (developer-only, excluded from CI)
- Backward compatibility regression suite for `.panda` file formats
- Parallel CTest execution

### 📚 Code Quality

- PascalCase standardization for all source files
- Scene, serialization, and module decomposition into single-responsibility components
- Procedural audio generation replacing embedded WAV assets
- Full Doxygen documentation setup
- Updated translations for 39 languages

### 📁 File Format

- **V4.4**: Wireless node mode
- **V4.5**: File-level metadata and embedded IC blob registry
- **V4.6**: Metadata map consolidation
- **V4.7**: QMap-based connection serialization
- Auto-migration from older formats with versioned backups

---

## 🙏 Contributors

- @darktorres — Primary development
- @GabrielGA-01 — SystemVerilog codegen
- @gbonavina — Arduino codegen
- @mams76 — Wireless Node Tx/Rx and Embedded ICs

---

**Full Changelog:** https://github.com/GIBIS-UNIFESP/wiRedPanda/compare/4.3.0...4.7.0
