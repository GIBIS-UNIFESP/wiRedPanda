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

## Phase 0 — Build System & Dependency Infrastructure ✅
**KDE Frameworks**: ECM (Extra CMake Modules)
**Prerequisite for**: All subsequent phases

### Goal
Integrate ECM into CMake so KDE Frameworks can be found and linked à la carte.

### Changes

**`CMakeLists.txt`**
```cmake
option(USE_KDE_FRAMEWORKS "Enable KDE Frameworks integration" OFF)

if(USE_KDE_FRAMEWORKS AND NOT EMSCRIPTEN)
    find_package(ECM REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
    include(KDEInstallDirs)
    include(KDECMakeSettings NO_POLICY_SCOPE)
    include(KDECompilerSettings NO_POLICY_SCOPE)
    # This project uses exceptions; re-enable via the ECM-provided function
    kde_enable_exceptions()
    message(STATUS "KDE Frameworks integration enabled (ECM ${ECM_VERSION})")
else()
    message(STATUS "KDE Frameworks integration disabled")
endif()
```

The option gates all ECM/KDE includes, allowing plain-Qt and KDE-enhanced builds from the same
source tree during transition. The option becomes the default ON once all phases are stable.
WASM always stays OFF — the `NOT EMSCRIPTEN` guard prevents ECM from being required there.

Per-phase `find_package(KF6 COMPONENTS ...)` blocks are added inside the same `if` block
as each phase is implemented.

### Compatibility fixes required

**1. Exceptions — `kde_enable_exceptions()`**

`KDECompilerSettings` disables exceptions globally by appending `-fno-exceptions` to
`CMAKE_CXX_FLAGS`. This project uses exceptions in the MCP server and several core utilities.
There is no skip variable to prevent the disabling upfront; the ECM-provided
`kde_enable_exceptions()` function is the correct opt-back-in mechanism — it strips
`-fno-exceptions` and `-DQT_NO_EXCEPTIONS` from `CMAKE_CXX_FLAGS` and appends `-fexceptions`.
Call it immediately after `include(KDECompilerSettings NO_POLICY_SCOPE)`.

**2. Symbol visibility — `nlohmann_json_schema_validator`**

`KDECompilerSettings` also sets `CMAKE_CXX_VISIBILITY_PRESET hidden`, which hides all symbols
by default. The `nlohmann_json_schema_validator` FetchContent library has no visibility
annotations, so all its symbols become hidden and the linker cannot resolve them from
`wiredpanda_lib`. Fix by restoring default visibility for that target after
`FetchContent_MakeAvailable`:

```cmake
set_target_properties(nlohmann_json_schema_validator PROPERTIES
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)
```

Note: `KDECompilerSettings` does **not** disable RTTI; no RTTI fix is needed.

### CMakePresets.json

Add a `kde` preset inheriting from `debug` with `USE_KDE_FRAMEWORKS=ON`, plus matching
build and test presets. This allows `cmake --preset kde` / `ctest --preset kde`.

### Platform guard pattern (only for OS-dependent frameworks)
```cpp
#ifndef Q_OS_WASM
  // KGlobalAccel, KCrash, KNotifications, KDBusAddons, KWindowSystem
#endif
```

Most frameworks (KConfig, KI18n, KWidgetsAddons, etc.) need no guard and work on WASM as-is.

### Verification
- `cmake --preset kde` configures without error, prints `KDE Frameworks integration enabled (ECM x.y.z)`
- `cmake --build --preset kde` produces a working binary
- `ctest --preset kde` — all tests pass
- `cmake --preset debug` (without flag) still builds pure-Qt version, prints `KDE Frameworks integration disabled`

---

## Phase 1 — Settings: QSettings → KConfig ✅
**KDE Frameworks**: `KF6::ConfigCore`
**Risk**: Low — fully encapsulated behind the existing `Settings` wrapper class

### Goal
Replace the QSettings backend with KConfig while keeping the `Settings` public API identical.
All 12 callers of `Settings::*()` require zero changes.

### Critical files
- `App/Core/Settings.h` — public API unchanged; include and private member swapped under `#ifdef`
- `App/Core/Settings.cpp` — implementation replaced

### CMake

```cmake
find_package(KF6Config REQUIRED)          # not "KF6 COMPONENTS Config" — KF6 uses per-module packages
add_compile_definitions(USE_KDE_FRAMEWORKS)
# after include(CMakeSources.cmake):
target_link_libraries(wiredpanda_lib PUBLIC KF6::ConfigCore)
```

Note: the target is `KF6::ConfigCore` (not `KF6::Config`). `KF6::ConfigGui` adds Qt Gui
dependencies and is only needed for color scheme / GUI config widgets.

### Settings.h

Swap includes under `#ifdef USE_KDE_FRAMEWORKS`:

```cpp
#ifdef USE_KDE_FRAMEWORKS
#include <KConfigGroup>
#include <KSharedConfig>
#else
#include <QSettings>
#endif
```

Move `QSettings *settings` and `settingsInstance()` behind `#ifndef USE_KDE_FRAMEWORKS`.
Add `static KConfigGroup groupFor(const QString &groupPath)` for the KDE path.

### Settings.cpp

Add a `groupFor()` helper that splits slash-separated group paths into nested `KConfigGroup` calls:

```cpp
KConfigGroup Settings::groupFor(const QString &groupPath)
{
    const QStringList parts = groupPath.split(u'/');
    KConfigGroup g = KSharedConfig::openConfig()->group(parts.constFirst());
    for (int i = 1; i < parts.size(); ++i) {
        g = g.group(parts.at(i));
    }
    return g;
}
```

Each typed accessor uses KConfig's typed `readEntry`/`writeEntry` overloads directly
inside `#ifdef USE_KDE_FRAMEWORKS` blocks. **Do not use the `QVariant` overload for
`QByteArray` fields** (window geometry) — KConfig's QVariant round-trip does not preserve
binary QByteArray content. Use `readEntry(key, QByteArray{})` and `writeEntry(key, ba)`
typed overloads instead.

Root-level QSettings keys (`fastMode`, `language`, `theme`, etc.) that have no group
in QSettings map to the `[General]` group in KConfig (`groupFor(u"General"_s)`).

Group mapping:
- `"MainWindow/geometry"` → `groupFor(u"MainWindow"_s).readEntry(u"geometry"_s, QByteArray{})`
- `"MainWindow/splitter/geometry"` → `groupFor(u"MainWindow/splitter"_s).readEntry(...)`
- `"fastMode"` → `groupFor(u"General"_s).readEntry(u"fastMode"_s, false)`
- `"updateCheck/lastCheckDate"` → `groupFor(u"updateCheck"_s).readEntry(...)`

`setHideV4Warning(false)` uses `g.deleteEntry(key)` instead of `QSettings::remove()`.

Add `using namespace Qt::StringLiterals;` at the top of Settings.cpp for `u"..."_s` literals.

**WASM fallback**: the `#else` branches retain the existing QSettings/localStorage path unchanged.

### Benefits gained
- Typed `readEntry`/`writeEntry` with declared defaults — no scattered magic strings
- Built-in config migration via `KConfigGroup::moveValuesTo()`
- Works identically on Windows, macOS, Linux

### Verification
- All existing settings persist across restarts
- `ctest --preset kde` — all 176 tests pass
- `ctest --preset debug` — all 176 tests pass (QSettings path unaffected)

---

## Phase 2 — Internationalization: Qt Linguist → KI18n ✅
**KDE Frameworks**: `KF6::I18n` (`find_package(KF6I18n REQUIRED)`, target `KF6::I18n`)
**Risk**: Medium — touches every translatable string, but mechanical

### Goal
Replace Qt's `tr()` with KDE's `i18n()` family for better plural forms, context strings,
and integration with KDE's translation infrastructure (Weblate/Phabricator).

### Critical files
- `pch.h` — i18n() compat shim for non-KDE builds
- `App/Core/Common.h` — PANDACEPTION macros
- `App/Main.cpp` — setApplicationDomain
- `App/UI/LanguageManager.cpp` — KDE locale path
- 27 App/ source files with `tr()` calls

### CMake

```cmake
find_package(KF6I18n REQUIRED)   # not "KF6 COMPONENTS I18n"
target_link_libraries(wiredpanda_lib PUBLIC KF6::I18n)
# Also add PCH reuse to test_utils and memory_helpers — they include headers
# that call i18n() in member initializers (e.g. ElementEditor.h)
target_precompile_headers(test_utils     REUSE_FROM wiredpanda_lib)
target_precompile_headers(memory_helpers REUSE_FROM wiredpanda_lib)
```

### pch.h — compat shim

Include `<KLocalizedString>` under `USE_KDE_FRAMEWORKS`; define a compat template for
non-KDE builds so the source files compile without `#ifdef` at every call site:

```cpp
#ifdef USE_KDE_FRAMEWORKS
#  include <KLocalizedString>
#else
template<typename... Args>
[[nodiscard]] inline QString i18n(const char *text, const Args &... args)
{
    QString s = QCoreApplication::translate("", text);
    if constexpr (sizeof...(args) > 0) {
        (..., (s = s.arg(args)));
    }
    return s;
}
#endif
```

The compat template uses `QCoreApplication::translate("", text)` with empty context —
translations that rely on class context won't resolve in non-KDE builds on this branch.
This is acceptable: the porting branch's `debug` preset is for development, not translation testing.

### String replacement

Global sed replaces both `ClassName::tr(` and bare `tr(` with `i18n(`:
```bash
find App/ -name "*.cpp" -o -name "*.h" | grep -v "Common\.h\|LanguageManager" \
  | xargs sed -i 's/[A-Za-z_][A-Za-z0-9_]*::tr(/i18n(/g; s/\btr(/i18n(/g'
```

Then absorb `.arg()` chains into inline arguments (`i18n("text %1").arg(x)` →
`i18n("text %1", x)`). A Python script handles nested parentheses in arguments:
`i18n("text %1 %2", a.method(), b->method())`.

The `PANDACEPTION` variadic macro in `Common.h` is updated separately:
```cpp
#ifdef USE_KDE_FRAMEWORKS
#  define PANDACEPTION(msg, ...) \
     Pandaception(i18n(msg __VA_OPT__(, __VA_ARGS__)), ...)
#else
#  define PANDACEPTION(msg, ...) \
     Pandaception(tr(msg) __VA_OPT__(.arg(__VA_ARGS__)), ...)
#endif
```

### App/Main.cpp

```cpp
#ifdef USE_KDE_FRAMEWORKS
KLocalizedString::setApplicationDomain("wiredpanda");
#endif
```
Call immediately after `app.setApplicationName()`.

### LanguageManager.cpp

KDE path calls `KLocalizedString::setLanguages({language})` instead of loading QTranslator.
The QTranslator path is preserved behind `#ifndef USE_KDE_FRAMEWORKS` for non-KDE builds.

**Translation files** (.po/.mo) are **not yet ported** — `KLocalizedString::setLanguages()`
is wired up but has no catalogs to load yet. Translations display in English on the KDE
build until `.po` files are generated from the existing `.ts` files.

### kde preset — Qt version conflict

The system KDE libraries (Ubuntu packages) are compiled against Qt 6.10. If the project
normally uses a private Qt installation (e.g. 6.9.x via `CMAKE_PREFIX_PATH` in the
environment), those versions are ABI-incompatible at runtime.

**Fix**: in `CMakePresets.json`, the `kde` preset:
1. Uses `build-kde/` as its own separate `binaryDir` (never shares with `build/`)
2. Explicitly sets `Qt6_DIR`, `Qt6Core_DIR`, etc. to the system Qt cmake paths
3. Sets `CMAKE_BUILD_RPATH` to `/usr/lib/x86_64-linux-gnu` to prepend system Qt libs
   first in RUNPATH, ensuring Qt 6.10 resolves before any private Qt in the environment

### Benefits gained
- Proper plural rules for all 39 supported languages
- Context strings eliminate ambiguous single-word translations
- Infrastructure in place for KDE's community translation workflow (.po files pending)

### Verification
- `ctest --preset kde` — all 176 tests pass
- `ctest --preset debug` — all 176 tests pass (QCoreApplication::translate path)
- Both presets build independently without corrupting each other's cmake cache

---

## Phase 3 — Application Identity: KAboutData ✅
**KDE Frameworks**: `KF6::CoreAddons` (`find_package(KF6CoreAddons REQUIRED)`, target `KF6::CoreAddons`)
**Risk**: Low

### Goal
Register proper application metadata used by KCrash, KNotifications, and the About dialog.

### Critical files
- `App/Main.cpp` — KAboutData construction inside `#ifdef USE_KDE_FRAMEWORKS`

### CMake

```cmake
find_package(KF6CoreAddons REQUIRED)
target_link_libraries(wiredpanda_lib PUBLIC KF6::CoreAddons)
```

### Main.cpp

Add `#include <KAboutData>` and `using namespace Qt::StringLiterals;` inside a
`#ifdef USE_KDE_FRAMEWORKS` guard at the top of Main.cpp (not in pch.h — Main.cpp
is compiled separately as the `wiredpanda` executable, not as part of `wiredpanda_lib`).

```cpp
#ifdef USE_KDE_FRAMEWORKS
{
    KAboutData about(
        u"wiredpanda"_s,
        i18n("wiRedPanda"),
        QString::fromLatin1(APP_VERSION),   // APP_VERSION is a narrow literal — use fromLatin1
        i18n("Educational digital logic circuit simulator"),
        KAboutLicense::GPL_V3,
        i18n("© 2015–2026 GIBIS-UNIFESP and the wiRedPanda contributors"),
        QString(),
        u"https://github.com/GIBIS-UNIFESP/wiRedPanda"_s
    );
    about.addAuthor(i18n("Fábio Augusto Menocci Cappabianco"), i18n("Lead Developer"), u"fcappabianco@gmail.com"_s);
    about.addAuthor(i18n("Rodrigo Torres"),                    i18n("Developer"),      u"torres.dark@gmail.com"_s);
    about.addAuthor(i18n("Davi Morales"),                      i18n("Developer"),      u"davimmorales@gmail.com"_s);
    about.addAuthor(i18n("Lucas Santana Lellis"),               i18n("Developer"),      u"lucaslellis777@gmail.com"_s);
    about.addAuthor(i18n("Vinícius Rodrigues Miguel"),         i18n("Developer"),      u"lemao.vrm07@hotmail.com"_s);
    KAboutData::setApplicationData(about);
    // setApplicationData calls KLocalizedString::setApplicationDomain(componentName)
    // internally; the explicit call is retained for clarity.
    KLocalizedString::setApplicationDomain("wiredpanda");
}
#endif
```

Place immediately after `app.setDesktopFileName()`. The existing `app.setOrganizationName()`,
`app.setApplicationName()` etc. calls are kept — `KAboutData::setApplicationData()` overrides
`applicationName` (to lowercase "wiredpanda") and `applicationVersion`, which is fine.

Note: do **not** use `u"" APP_VERSION ""_s` — adjacent string literal concatenation between
UTF-16 (`u""`) and narrow (`APP_VERSION`) literals is ill-formed. Use `QString::fromLatin1(APP_VERSION)`.

### Benefits gained
- Feeds automatically into KCrash (Phase 7) and KNotifications (Phase 7)
- About dialog available for free via `KAboutData::applicationData()`
- Application registered with the KDE session

### Verification
- `ctest --preset kde` — all 176 tests pass
- `ctest --preset debug` — all 176 tests pass (non-KDE path unaffected)

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

### 5a — Message Dialogs: QMessageBox → KMessageBox ✅
**KDE Frameworks**: `KF6::WidgetsAddons`
**Files modified** — wrapper + ~25 call-site replacements:
- New `App/UI/MessageDialog.h` — thin namespace wrapper around QMessageBox/KMessageBox
- `App/Main.cpp`, `App/Core/Application.cpp`, `App/Scene/Workspace.cpp`,
  `App/UI/ElementEditor.cpp`, `App/UI/MainWindow.cpp`,
  `App/BeWavedDolphin/BeWavedDolphin.cpp` — call sites
- `Tests/Unit/Ui/TestDialogs.cpp`, `Tests/Integration/TestMainWindowGui.cpp` —
  test fixtures updated to poll for the modal dialog (KMessageBox defers show
  past `singleShot(0)`)

The wrapper exposes `MessageDialog::error / information / warning / questionYesNo`
with `(parent, text, title)` signature. KDE builds use KMessageBox; non-KDE
builds fall back to QMessageBox. Multi-button dialogs (Save/Discard/Cancel,
Yes/YesToAll/No/NoToAll/Cancel) and `about` / `aboutQt` keep QMessageBox
since KMessageBox has no direct equivalent.

### 5b — Global Shortcuts: KGlobalAccel ✅
**KDE Frameworks**: `KF6::GlobalAccel` (no extra dependency — pulled in via Keys flag)

Re-enabled `KXmlGuiWindow::Keys` flag by switching `setupGUI()` to
`KXmlGuiWindow::Default` (= `Create | StatusBar | ToolBar | Save | Keys`)
in both `MainWindow` and `BewavedDolphin`.

**Investigation correction**: A previous note claimed the `Keys` flag hung
headless tests via D-Bus. That was wrong. The actual hangs in Phase 4 came
entirely from `checkAmbiguousShortcuts()` opening a `KMessageBox::information`
modal dialog because of the KCommandBar (`Ctrl+Alt+I`) and KHelpMenu (`F1`)
shortcut conflicts — both already fixed in Phase 4 via `setCommandBarEnabled(false)`
and `setHelpMenuEnabled(false)`.

A standalone test (`/tmp/kga_test`) confirms KGlobalAccel handles a missing
`org.kde.kglobalaccel` D-Bus service gracefully: `setShortcut()` returns in
~40 ms after one D-Bus auto-activation timeout, prints a stderr error, and
moves on. No hang. With the daemon present (KDE Plasma session), shortcuts
become user-rebindable via System Settings → Shortcuts.

### 5c — Recent Files: KRecentFilesAction ✅
**KDE Frameworks**: `KF6::ConfigWidgets` (already pulled in transitively via XmlGui)

KDE builds use `KStandardAction::openRecent` returning a `KRecentFilesAction`.
The action is added to the action collection before `setupGUI()` so the
`<Action name="file_open_recent"/>` reference in `wiredpandaui.rc` resolves
into the File menu automatically. URLs persist via `KSharedConfig::openConfig()`
under group `RecentFiles`. `loadEntries`/`saveEntries` happen on startup and
on each new file save (via the existing `addRecentFile` signal).

Non-KDE builds keep the existing `App/IO/RecentFiles` + manual menu population
under `#ifndef USE_KDE_FRAMEWORKS`. The `m_ui->menuRecentFiles` pointer holds
either the KRecentFilesAction's auto-built menu (KDE) or the manually-populated
QMenu (non-KDE).

**Behavioral note**: `KRecentFilesAction::addUrl` silently drops URLs under
`QDir::tempPath()`. Tests that exercised the recent-files plumbing via a
`QTemporaryDir` fixture were updated: KDE path verifies the action is wired
and the menu exists; non-KDE path keeps the full save → reload-via-menu check.

### 5c (FileDialog part) — SKIPPED ⏭️

**Reason for skip**: When wiRedPanda runs on KDE Plasma, the
`kdeplatformfiledialog` plugin already replaces `QFileDialog` with KIO's
`KFileWidget` transparently. KF6 itself dropped the standalone `KFileDialog`
class — the recommended approach is `QFileDialog` + the platform plugin.
Replacing `RealFileDialogProvider` would add boilerplate without user-visible
change (and pulling in `KIOFileWidgets` adds a substantial dependency for
zero benefit when the platform plugin already routes through it).

---

## Phase 6 — Theming: KColorScheme + KSvg + KIconThemes

### 6a — Color Scheme: KColorScheme → integrate into ThemeManager ✅
**KDE Frameworks**: `KF6::ColorScheme` (`find_package(KF6ColorScheme REQUIRED)`)
**Files modified**:
- `App/Core/ThemeManager.cpp` — KDE-only `Theme::System` branch in
  `ThemeAttributes::setTheme()` queries `KColorScheme(View)` and
  `KColorScheme(Selection)` for scene background, foreground, selection,
  semantic connection states (Inactive/Neutral/Positive/Negative/Link foregrounds),
  port output (LinkText), and port hover (decoration HoverColor).
- `themePath()` and `effectiveTheme()` derive Light/Dark from
  `KColorScheme(View).background().lightness()` for KDE+System mode so
  resource-path lookups (`:/Components/Memory/{Light,Dark}/...`) still pick
  the right variant.
- `setTheme()` and the constructor pass `Theme::System` through unresolved on
  KDE so `ThemeAttributes` can route to the KColorScheme path; on non-KDE
  builds, System still resolves to Light/Dark up-front (unchanged).
- `onSystemColorSchemeChanged()` re-queries KColorScheme when in System mode.

`Theme::Light` and `Theme::Dark` keep their hardcoded palette and scene colors
so users still get a deterministic look when explicitly choosing one. With
`Theme::System` (the default), the canvas now blends with the user's
KDE/Plasma color scheme — palette is left untouched (Plasma owns it) and
scene colors come from KColorScheme tokens.

### 6b — SVG Icons: KSvg ⏭️ SKIPPED
The roadmap proposed re-coloring 196 component SVGs at runtime via KSvg. In
practice the project only ships dual variants for 8 memory components
(`Components/Memory/{Light,Dark}/`); the remaining ~183 SVGs are intentionally
multi-coloured (red/green ports, yellow LEDs, etc.) and don't fit KSvg's
stylesheet-based monochrome re-colouring model without redesigning each asset.
Skipping yields no user-visible regression and avoids a substantial pixel-art
refactor with unclear payoff.

### 6c — Icon Themes: theme-named icons via QIcon::fromTheme ✅
**KDE Frameworks**: `KF6::IconThemes` (`find_package(KF6IconThemes REQUIRED)` —
links the runtime icon engine; bundled-resource fallbacks remain in place)
**Files modified**:
- `App/UI/MainWindow.cpp` — `setupKdeActions()`'s `addAction` lambda gained
  an optional `themeName` parameter that resolves to
  `QIcon::fromTheme(themeName, QIcon(fallbackPath))`. Applied to: reload
  (`view-refresh`), delete (`edit-delete`), rename (`edit-rename`), rotate
  left/right (`object-rotate-left/right`), flip h/v (`object-flip-horizontal/vertical`),
  clear selection (`edit-clear`), reset zoom (`zoom-original`), restart
  (`media-playback-stop`), about (`help-about`).
- `App/BeWavedDolphin/BeWavedDolphin.cpp` — same lambda extended; applied to
  clear (`edit-clear-all`), fit screen (`zoom-fit-best`), reset zoom
  (`zoom-original`), about (`help-about`).
- KStandardAction (Phase 4) already wires Breeze icons for new/open/save/save-as/quit/cut/copy/paste/select-all/zoom-in/zoom-out, so those needed no change.
- Two-state toggle icons (Play/Pause, Mute/Unmute) keep bundled SVGs because
  `QIcon::On`/`QIcon::Off` states can't be resolved from two separate theme
  names within a single `QIcon`.

### Verification
- `ctest --preset kde` — all 176 tests pass
- `ctest --preset debug` — all 175 tests pass (non-KDE path unchanged)

---

## Phase 7 — System Integration: KCrash + KNotifications

### 7a — Crash Reporting: KCrash ✅
**KDE Frameworks**: `KF6::Crash` (Linux only — `find_package(KF6Crash REQUIRED)` is
gated on `UNIX AND NOT APPLE` so macOS and Windows skip it)
**Files modified**: `App/Main.cpp`

`KCrash::initialize()` runs immediately after `KAboutData::setApplicationData()`
inside the existing `USE_KDE_FRAMEWORKS` block, behind a nested `Q_OS_LINUX`
guard. KCrash picks up the application identity from KAboutData, so DrKonqi
(when present) presents a familiar crash dialog on KDE Plasma. Sentry's
Crashpad backend remains the primary handler on all platforms; KCrash is
purely additive on Linux. The roadmap suggested calling
`KCrash::setDrKonqiEnabled(true)` — that helper was dropped in KF6
(DrKonqi is the default sink), so just `initialize()` is needed.

### 7b — Notifications: KNotifications ✅
**KDE Frameworks**: `KF6::Notifications`
**Files modified**:
- `App/UI/MainWindow.cpp` — `showUpdateDialog()` got a KDE branch that emits a
  persistent `KNotification` with `Download` (or `View Release`) and
  `Skip This Version` action buttons. Closing the notification still records
  `Settings::setUpdateCheckLastDate`. Non-KDE keeps the existing modal QDialog.
- `App/Resources/KDE/wiredpanda.notifyrc` — declares the `updateAvailable`
  event with persistent flag and `system-software-update` icon; installed via
  `${KDE_INSTALL_KNOTIFYRCDIR}` so KNotification can resolve per-event metadata.

### Verification
- `ctest --preset kde` — all 176 tests pass
- `ctest --preset debug` — all 175 tests pass (non-KDE QDialog path unchanged)

---

## Phase 8 — KNewStuff: Circuit Marketplace ✅
**KDE Frameworks**: `KF6::NewStuffWidgets` (`find_package(KF6NewStuff REQUIRED)`,
target `KF6::NewStuffWidgets` — pulls in `KF6::NewStuffCore` transitively)
**Files modified**:
- `App/UI/MainWindow.cpp` — `setupKdeActions()` registers a
  `wiredpanda_download_circuits` action (icon: `get-hot-new-stuff`) wired to
  `MainWindow::downloadCircuits()`. The handler creates a `KNSWidgets::Dialog`
  with `wiredpanda.knsrc`, opens it non-modally, and on close iterates
  `changedEntries()`, opening each newly installed `.panda` file via
  `loadPandaFile()`.
- `App/UI/MainWindow.h` — added `void downloadCircuits()` declaration inside the
  existing `#ifdef USE_KDE_FRAMEWORKS` block.
- `App/Resources/KDE/wiredpandaui.rc` — `<Action name="wiredpanda_download_circuits"/>`
  inserted into the File menu after `file_open_recent` so it appears alongside
  the other open-circuit entries.
- `App/Resources/KDE/wiredpanda.knsrc` (NEW) — GHNS config pointing at
  `https://autoconfig.kde.org/ocs/providers.xml` with `Categories=wiRedPanda Circuits`,
  `TargetDir=wiredpanda/circuits`, `Uncompress=archive`, and a content warning so
  users know to trust their sources before opening downloaded circuits.
- `CMakeLists.txt` — `find_package(KF6NewStuff REQUIRED)` and
  `KF6::NewStuffWidgets` linked into `wiredpanda_lib`. Install rule added for
  `wiredpanda.knsrc` under `${KDE_INSTALL_KNSRCDIR}` so `KNSWidgets::Dialog` can
  resolve the file by short name.

### Server endpoint — pending
The action is wired and the dialog opens cleanly, but no Pling Store project
exists yet for wiRedPanda. Until a project is registered (or a self-hosted
GHNS endpoint is configured) and the `Categories` field is updated to match
the live category name, the dialog will display zero entries. This is purely
a server-side setup; the client implementation is complete.

### Verification
- `ctest --preset kde` — all 176 tests pass
- `ctest --preset debug` — all 175 tests pass (non-KDE path unchanged)

---

## Phase 9 — KPlotting: Waveform Viewer
**KDE Frameworks**: `KF6::Plotting`

Replace the custom `BeWavedDolphin` waveform rendering (QStandardItemModel + custom delegate)
with KPlotting's lightweight plot widget. Reduces ~400 lines of custom painting code.

---

## Phase 10 — ThreadWeaver: Parallel Simulation
**KDE Frameworks**: `KF6::ThreadWeaver`

Break the fixed 1ms `QTimer`-driven simulation loop into a ThreadWeaver job graph.
Each topological layer of the circuit becomes a parallel job, enabling multi-core simulation
of large circuits. The `Simulation.cpp` architecture is well-suited for this — topological
ordering already exists; parallelism is the only missing piece.

---

## Phase 11 — Purpose: Circuit Sharing
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

---

## Phase 12 — KParts: Dolphin/Okular Integration
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
    └── Phase 8 (KNewStuff)             ← independent, after Phase 4
    └── Phase 9 (KPlotting)             ← independent, after Phase 4
    └── Phase 10 (ThreadWeaver)         ← independent, after Phase 4
    └── Phase 11 (Purpose)              ← independent, after Phase 4
    └── Phase 12 (KParts)               ← independent, after Phase 4
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
