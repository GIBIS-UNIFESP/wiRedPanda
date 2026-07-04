# KDE Frameworks Port — Roadmap for wiRedPanda

## Context

wiRedPanda is a Qt6 educational circuit simulator targeting Windows, macOS, Linux, and WebAssembly.
This roadmap describes a phased adoption of KDE Frameworks that delivers progressive improvements
without requiring a single big-bang rewrite. Each phase is independently shippable.

The strategy is **additive by default**: KDE Frameworks are available on all desktop platforms,
so adopting them does not break Windows/macOS support.

### WASM compatibility

KDE Frameworks are C++/Qt addons with no fundamental barrier to WASM compilation. Compatibility
is per-framework based on whether each library touches OS-level services:

**Compatible with WASM** (pure Qt/C++, no OS services):
- KConfig, KConfigCore, KConfigWidgets
- KI18n
- KCoreAddons
- KWidgetsAddons, KGuiAddons
- KItemModels, KItemViews
- KColorScheme

**Requires `#ifdef Q_OS_WASM` guard** (depends on OS services absent in WASM):
- KGlobalAccel — OS-level shortcut registration
- KCrash — OS signal handling (SIGSEGV etc.)
- KNotifications — OS notification daemon
- KDBusAddons — D-Bus does not exist in WASM
- KWindowSystem — talks to X11/Wayland/Win32 directly

**Verify per use case**:
- KIO — local file dialog paths may work; async job system and networking likely don't

---

## Phase 0 — Build System & Dependency Infrastructure
**KDE Frameworks**: ECM (Extra CMake Modules)
**Prerequisite for**: All subsequent phases

### Goal
Integrate ECM into CMake so KDE Frameworks can be found and linked à la carte.

### Changes

**`CMakeLists.txt`**
```cmake
find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
```

Then add per-phase `find_package(KF6 COMPONENTS ...)` blocks, each gated behind an opt-in CMake option:
```cmake
option(USE_KDE_FRAMEWORKS "Enable KDE Frameworks integration" OFF)
```

This allows the project to ship both a plain-Qt build and a KDE-enhanced build from the same
source tree during transition. The option becomes the default ON once all phases are stable.

### Platform guard pattern (only for OS-dependent frameworks)
```cpp
#ifndef Q_OS_WASM
  // KGlobalAccel, KCrash, KNotifications, KDBusAddons, KWindowSystem
#endif
```

Most frameworks (KConfig, KI18n, KWidgetsAddons, etc.) need no guard and work on WASM as-is.

### Verification
- `cmake --preset debug -DUSE_KDE_FRAMEWORKS=ON` configures without error
- `cmake --build --preset debug` produces a working binary
- `cmake --preset debug` (without flag) still builds pure-Qt version

---

## Phase 1 — Settings: QSettings → KConfig
**KDE Frameworks**: `KF6::Config`, `KF6::ConfigCore`
**Risk**: Low — fully encapsulated behind the existing `Settings` wrapper class

### Goal
Replace the QSettings backend with KConfig while keeping the `Settings` public API identical.
All 12 callers of `Settings::*()` require zero changes.

### Critical files
- `App/Core/Settings.h` (87 lines) — public API, unchanged
- `App/Core/Settings.cpp` (206 lines) — implementation replaced

### Changes

**`App/Core/Settings.cpp`**

Replace the private `QSettings` instance with `KSharedConfig` + `KConfigGroup`:

```cpp
// Before
static QSettings &instance() {
    static QSettings s(QSettings::IniFormat, QSettings::UserScope,
                       "GIBIS-UNIFESP", "wiRedPanda");
    return s;
}

// After (non-WASM)
#ifndef Q_OS_WASM
static KConfigGroup group(const QString &name) {
    return KSharedConfig::openConfig()->group(name);
}
#endif
```

Each accessor migrates from `settings.value(key, default)` to `cfg.readEntry(key, default)`.
Group structure maps naturally: `mainWindow/geometry` → group("mainWindow").readEntry("geometry").

**`.kcfg` schema file** (new, optional but recommended)
`App/Resources/wiredpanda.kcfg` — declares all settings with types and defaults.
Enables KConfigXT code generation for compile-time-checked settings access in the future.

**WASM fallback**: `Q_OS_WASM` block retains existing QSettings/localStorage path unchanged.

### Benefits gained
- Typed `.kcfg` schema with declared defaults — no more scattered magic strings
- Built-in config migration via `KConfigGroup::moveValuesTo()`
- Works identically on Windows, macOS, Linux

### Verification
- All existing settings persist across restarts
- `~/.config/wiRedPandarc` (Linux) or `%APPDATA%\wiRedPanda\wiRedPandarc` (Windows) created
- Run full test suite: `ctest --preset debug`

---

## Phase 2 — Internationalization: Qt Linguist → KI18n
**KDE Frameworks**: `KF6::I18n`
**Risk**: Medium — touches every translatable string, but mechanical

### Goal
Replace Qt's `tr()` with KDE's `i18n()` family for better plural forms, context strings,
and integration with KDE's translation infrastructure (Weblate/Phabricator).

### Critical files
- `App/UI/LanguageManager.h` (69 lines)
- `App/UI/LanguageManager.cpp`
- Every `.cpp` file using `tr()` — project-wide change (~187 files)
- `App/Resources/Translations.qrc`

### Changes

**Macro replacement** (global search-replace, then manual review of plurals):
```cpp
tr("Save File")           → i18n("Save File")
tr("%1 items").arg(n)     → i18np("1 item", "%1 items", n)   // plural-aware
tr("Context", "text")     → i18nc("Context", "text")          // disambiguated
```

**`App/UI/LanguageManager.cpp`**
Remove `QTranslator` loading logic. KI18n handles locale detection automatically via
`KLocalizedString::setApplicationDomain("wiredpanda")` called once at startup.

**`App/Main.cpp`**
```cpp
KLocalizedString::setApplicationDomain("wiredpanda");
```

**Translation files**: Migrate from Qt `.ts`/`.qm` format to GNU Gettext `.po`/`.mo`.
ECM provides `ecm_create_qm_loader()` for the transition period if needed.

**`CMakeLists.txt`**
```cmake
find_package(KF6 COMPONENTS I18n REQUIRED)
ki18n_install(po)
```

### Benefits gained
- Proper plural rules for all 39 supported languages (Qt's plural support is limited)
- Context strings eliminate ambiguous single-word translations
- Integration with KDE's community translation workflow

### Verification
- Switch language in Settings → all UI strings translate correctly
- Test a plural-heavy string with n=0, 1, 2 in a language with complex plural rules (Polish/Russian)
- `ctest --preset debug`

---

## Phase 3 — Application Identity: KAboutData
**KDE Frameworks**: `KF6::CoreAddons` (KAboutData, KAboutPerson)
**Risk**: Low

### Goal
Register proper application metadata used by KCrash, KNotifications, and the About dialog.

### Critical files
- `App/Main.cpp` (380 lines)
- `App/Core/Application.h` / `App/Core/Application.cpp`

### Changes

**`App/Main.cpp`**
```cpp
KAboutData about(
    "wiredpanda",                          // component name
    i18n("wiRedPanda"),                    // display name
    PROJECT_VERSION,                       // version string
    i18n("Digital logic simulator"),       // short description
    KAboutLicense::GPL_V3,
    i18n("© 2020–2025 GIBIS-UNIFESP"),
    QString(),
    "https://github.com/GIBIS-UNIFESP/wiRedPanda"
);
about.addAuthor(i18n("Fábio Cappabianco"), i18n("Lead Developer"), "...");
KAboutData::setApplicationData(about);
```

The existing `Application` class can keep `QApplication` as base for now;
`KAboutData` does not require `KApplication`.

### Benefits gained
- Feeds automatically into KCrash (Phase 6) and KNotifications (Phase 5)
- About dialog available for free via `KAboutData::applicationData()`
- Application registered with the KDE session

---

## Phase 4 — Main Window: KXmlGuiWindow + KActionCollection
**KDE Frameworks**: `KF6::XmlGui`
**Risk**: High — largest structural change in the port

### Goal
Replace the hand-written `MainWindowUI` setup (813 lines) with KXmlGui's XML-driven
menu/toolbar system. Users gain the ability to customize toolbar layout and save it per-profile.
All 44 QActions move into a `KActionCollection` for centralized management.

### Critical files
- `App/UI/MainWindow.h` (318 lines) — base class changes
- `App/UI/MainWindow.cpp` (2102 lines) — action connections
- `App/UI/MainWindowUI.h` / `App/UI/MainWindowUI.cpp` (813 lines) — replaced by XML
- `App/BeWavedDolphin/BeWavedDolphin.h` / `.cpp` (1338 lines) — second main window
- New file: `App/Resources/wiredpandaui.rc`

### Step-by-step

**Step 4a — Create `wiredpandaui.rc`** (the XML menu/toolbar definition)
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE gui SYSTEM "kpartgui.dtd">
<gui name="wiredpanda" version="1">
  <MenuBar>
    <Menu name="file"><text>&amp;File</text>
      <Action name="file_new"/>
      <Action name="file_open"/>
      <Separator/>
      <Action name="file_save"/>
      <Action name="file_save_as"/>
      ...
    </Menu>
    <Menu name="edit"><text>&amp;Edit</text>
      <Action name="edit_undo"/>
      <Action name="edit_redo"/>
      ...
    </Menu>
    ...
  </MenuBar>
  <ToolBar name="mainToolBar"><text>Main Toolbar</text>
    <Action name="file_new"/>
    <Action name="file_open"/>
    <Action name="file_save"/>
    <Separator/>
    <Action name="edit_undo"/>
    <Action name="edit_redo"/>
    ...
  </ToolBar>
</gui>
```

**Step 4b — Change `MainWindow` base class**
```cpp
// Before
class MainWindow : public QMainWindow

// After
class MainWindow : public KXmlGuiWindow
```

**Step 4c — Migrate actions into `KActionCollection`**

Replace all `new QAction(...)` with `KActionCollection`:
```cpp
// Before (MainWindowUI.cpp)
m_actionNew = new QAction(QIcon(":/..."), tr("New"), this);
m_actionNew->setShortcut(QKeySequence::New);

// After (MainWindow constructor)
auto *actionNew = KStandardAction::openNew(this, &MainWindow::newFile, actionCollection());
// KStandardAction handles icon, text, and shortcut automatically
```

Use `KStandardAction` for all standard operations (New, Open, Save, SaveAs, Quit, Undo, Redo,
Cut, Copy, Paste, SelectAll, ZoomIn, ZoomOut, FullScreen). Custom actions via:
```cpp
auto *action = actionCollection()->addAction("simulation_play");
action->setText(i18n("Play Simulation"));
action->setIcon(QIcon::fromTheme("media-playback-start"));
actionCollection()->setDefaultShortcut(action, Qt::Key_F5);
```

**Step 4d — Replace `setupUi()` with `setupGUI()`**
```cpp
// Before (end of MainWindow constructor)
m_ui->setupUi(this);

// After
setupGUI(Default, "wiredpandaui.rc");
```

**Step 4e — BeWavedDolphin** follows the same pattern as a secondary `KXmlGuiWindow`.

### Benefits gained
- Users can right-click toolbar to add/remove actions
- Toolbar layout persists per-user automatically (no manual save/restore code)
- `KStandardAction` provides correct icons and shortcuts per platform
- `KActionCollection` enables global shortcut rebinding (feeds Phase 5b)

### Verification
- All menus and toolbars appear with correct actions
- Toolbar customization dialog opens (right-click toolbar → Configure Toolbars)
- Undo/redo shortcuts work
- Window state persists across restarts
- `ctest --preset debug`

---

## Phase 5 — Dialogs & File Handling
**KDE Frameworks**: `KF6::WidgetsAddons`, `KF6::KIO`, `KF6::JobWidgets`

### 5a — Message Dialogs: QMessageBox → KMessageBox
**Files**: All files with `#include <QMessageBox>` (~14 call sites across 6 files)

```cpp
// Before
QMessageBox::warning(this, tr("Warning"), tr("File not found"));

// After
KMessageBox::sorry(this, i18n("File not found"));
```

Mapping:
- `QMessageBox::warning` → `KMessageBox::sorry` or `KMessageBox::error`
- `QMessageBox::question` → `KMessageBox::questionTwoActions`
- `QMessageBox::information` → `KMessageBox::information`
- `QMessageBox::critical` → `KMessageBox::error`

### 5b — Global Shortcuts: KGlobalAccel
**KDE Frameworks**: `KF6::GlobalAccel`
**Files**: `App/UI/MainWindow.cpp` (all `actionCollection()->setDefaultShortcut()` calls)

After Phase 4 (KActionCollection), wire global shortcuts:
```cpp
KGlobalAccel::setGlobalShortcut(actionSimulationPlay, QKeySequence(Qt::CTRL | Qt::Key_F5));
```

Users can rebind via **System Settings → Shortcuts → wiRedPanda**.

### 5c — File Dialogs: KIO::FileDialog + KRecentFilesAction
**Files**: `App/UI/FileDialogProvider.cpp`, `App/IO/RecentFiles.h/cpp`, `App/UI/MainWindowUI.cpp`

**`FileDialogProvider`** — replace `RealFileDialogProvider` implementation:
```cpp
// Before
QString path = QFileDialog::getOpenFileName(parent, caption, dir, filter);

// After
QString path = KFileDialog::getOpenFileName(KUrl(dir), filter, parent, caption);
// or with KIO:
auto *dialog = new KOpenFileDialog(parent);
dialog->setMimeTypeFilters({"application/x-wiredpanda", "image/png"});
```

**`RecentFiles`** — replace the hand-rolled MRU list with `KRecentFilesAction`:
```cpp
// Remove RecentFiles.h/cpp entirely.
// In MainWindow:
auto *recentFiles = new KRecentFilesAction(i18n("Open &Recent"), actionCollection());
actionCollection()->addAction("file_open_recent", recentFiles);
connect(recentFiles, &KRecentFilesAction::urlSelected, this, &MainWindow::openUrl);
recentFiles->loadEntries(KSharedConfig::openConfig()->group("RecentFiles"));
```

This also removes the `QFileSystemWatcher` used to detect deleted files — KIO handles this.

### Verification
- Open/save dialogs work on all platforms
- Recent files menu populates and opens files correctly
- File-not-found entries are removed automatically

---

## Phase 6 — Theming: KColorScheme + KSvg + KIconThemes
**KDE Frameworks**: `KF6::ColorScheme`, `KF6::Svg`, `KF6::IconThemes`

### 6a — Color Scheme: KColorScheme → replace ThemeManager
**Files**: `App/Core/ThemeManager.h/cpp`

```cpp
// Current: manual light/dark palette construction
// After: query KColorScheme for semantic color tokens
KColorScheme scheme(QPalette::Active, KColorScheme::View);
QColor background = scheme.background().color();
QColor foreground = scheme.foreground().color();
QColor highlight  = scheme.decoration(KColorScheme::HoverColor).color();
```

The app automatically follows the OS/KDE global color scheme. The manual "Light/Dark" toggle
in Settings can become "Follow System" by default with override option.

### 6b — SVG Icons: KSvg
**Files**: All files loading SVG icons from resources (toolbar icons, component icons)

KSvg enables stylesheet-based re-coloring of SVGs, so the 196 bundled SVG component icons
automatically adapt to the active color scheme without maintaining separate light/dark variants.

```cpp
// Before: QIcon(":/Components/and-gate.svg")  — static color
// After:
KSvg::SvgIcon icon(":/Components/and-gate.svg");
icon.setColor(KColorScheme(QPalette::Active).foreground().color());
```

### 6c — Icon Themes: KIconThemes
**Files**: `App/UI/MainWindowUI.cpp` (toolbar icon assignments)

Replace hardcoded resource paths with theme-aware icon names where Breeze provides equivalents:
```cpp
// Before: QIcon(":/Interface/Toolbar/file-new.svg")
// After:  QIcon::fromTheme("document-new")
```

Keep resource fallback for custom icons (circuit component SVGs) that have no theme equivalent.

### Verification
- Switch OS to dark mode → app follows automatically
- Component SVGs render with correct foreground color
- Standard toolbar icons use Breeze theme on Linux, fallback SVGs on Windows/macOS

---

## Phase 7 — System Integration: KCrash + KNotifications
**KDE Frameworks**: `KF6::Crash`, `KF6::Notifications`

### 7a — Crash Reporting: KCrash
**Files**: `App/Main.cpp` (Sentry initialization block)

On Linux, complement Sentry Crashpad with KCrash/DrKonqi:
```cpp
#ifdef Q_OS_LINUX
KCrash::initialize();
KCrash::setDrKonqiEnabled(true);
#endif
```

Sentry remains the primary backend on all platforms. KCrash adds DrKonqi as a secondary
reporter on Linux, giving KDE users a familiar crash dialog.

### 7b — Notifications: KNotifications
**Files**: `App/Core/UpdateChecker.cpp` (the `updateAvailable` signal handler in `MainWindow.cpp`)

Replace the current update modal with a passive system notification:
```cpp
// Before: QMessageBox::information(this, tr("Update available"), ...);

// After:
auto *notification = new KNotification("updateAvailable", KNotification::Persistent, this);
notification->setTitle(i18n("wiRedPanda Update Available"));
notification->setText(i18n("Version %1 is available. Click to download.", version));
notification->setActions({i18n("Download"), i18n("Skip")});
connect(notification, &KNotification::action1Activated, this, [url]{ QDesktopServices::openUrl(url); });
notification->sendEvent();
```

Add `App/Resources/wiredpanda.notifyrc` to declare notification categories.

### Verification
- Force an update check → notification appears in system notification area
- Clicking "Download" opens browser
- On crash (intentional test crash) → DrKonqi dialog appears on Linux

---

## Phase 8 — Enhanced Features (Optional / Future)

These are independent features that can be adopted in any order after Phase 4 is complete.

### 8a — KNewStuff: Circuit Marketplace
**KDE Frameworks**: `KF6::NewStuff`

Allow users to download/share circuits and IC libraries from a hosted GHNS (Get Hot New Stuff) store.
Requires setting up a `wiredpanda.knsrc` config and a server endpoint (Pling Store or self-hosted).

```cpp
auto *dlg = new KNSWidgets::Dialog("wiredpanda.knsrc", this);
dlg->open();
connect(dlg, &KNSWidgets::Dialog::accepted, this, &MainWindow::loadDownloadedCircuits);
```

### 8b — KPlotting: Waveform Viewer
**KDE Frameworks**: `KF6::Plotting`

Replace the custom `BeWavedDolphin` waveform rendering (QStandardItemModel + custom delegate)
with KPlotting's lightweight plot widget. Reduces ~400 lines of custom painting code.

### 8c — ThreadWeaver: Parallel Simulation
**KDE Frameworks**: `KF6::ThreadWeaver`

Break the fixed 1ms `QTimer`-driven simulation loop into a ThreadWeaver job graph.
Each topological layer of the circuit becomes a parallel job, enabling multi-core simulation
of large circuits. The `Simulation.cpp` architecture is well-suited for this — topological
ordering already exists; parallelism is the only missing piece.

### 8d — Purpose: Circuit Sharing
**KDE Frameworks**: `KF6::Purpose`

Add a "Share" toolbar action that lets users send `.panda` files via email, Nextcloud,
Telegram, or any Purpose-registered handler:
```cpp
auto *shareMenu = new Purpose::Menu(this);
shareMenu->model()->setInputData(QJsonObject{
    {"url",      QUrl::fromLocalFile(currentFile).toString()},
    {"mimeType", "application/x-wiredpanda"}
});
```

### 8e — KParts: Dolphin/Okular Integration
**KDE Frameworks**: `KF6::Parts`

Expose a read-only `KPart` so `.panda` files render as circuit previews inside Dolphin
(file manager) and Okular (document viewer) without opening the full application.

---

## Dependency Graph

```
Phase 0 (ECM)
    └── Phase 1 (KConfig)
    └── Phase 3 (KAboutData)
            └── Phase 4 (KXmlGui)        ← largest change
                    └── Phase 5 (Dialogs/KIO)
                    └── Phase 5b (KGlobalAccel)
    └── Phase 2 (KI18n)                  ← independent, can run parallel to Phase 1
    └── Phase 6 (Theming)               ← independent after Phase 0
    └── Phase 7 (KCrash/KNotifications) ← independent after Phase 3
    └── Phase 8a–e                       ← independent, after Phase 4
```

---

## Files Unchanged Throughout (no KDE changes needed)

- All element classes (`App/Element/`) — ~60 files
- All simulation logic (`App/Simulation/`) — ~15 files
- All scene/graphics code (`App/Scene/`) except clipboard MIME types — ~20 files
- All serialization/IO (`App/IO/`) except RecentFiles — ~15 files
- All code generation (`App/CodeGen/`) — ~10 files
- All tests (`Tests/`) — Qt-based tests remain valid
- `App/Resources/Components/` SVGs — used as-is (Phase 6 adds re-coloring on top)

---

## Migration Strategy: Opt-in CMake Flag

During the transition, maintain `USE_KDE_FRAMEWORKS=OFF` as the default. CI builds both variants:

```yaml
# .github/workflows/build.yml additions
- name: Build with KDE Frameworks
  run: cmake --preset debug -DUSE_KDE_FRAMEWORKS=ON && cmake --build --preset debug
```

Flip the default to `ON` after Phase 5 is complete and all desktop platforms verified.
WASM build always uses `USE_KDE_FRAMEWORKS=OFF`.
