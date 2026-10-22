# The Qt 6.8 floor

The project's minimum Qt goes from 6.2 to 6.8, for every build, in one small pull request of its own. `QRhiWidget`,
which plan 10's GPU canvas draws in, first appeared in Qt 6.7, and the user kept one floor for the whole project rather
than one for GPU builds only; the floor is 6.8, not 6.7, since no maintained distribution ships 6.7, Qt's support for
6.7 has ended, and 6.8 is a long-term-support release that CI already builds as its oldest Qt. The pull request raises
the number CMake asks for, removes the code that exists only for Qt older than 6.8, says 6.8 in every document that
names a minimum, and makes the documented way to install Qt on an older distribution work. It adds no CI build: the
6.8.3 jobs and the i386 job's 6.8.2 already test the floor. It comes right after plan 2.

- **Depends on:** plan 2, landed, since the plans run one at a time, in their order (the plans' rules, §1); and plan
  1's translatable-strings target (plan 1's §3.13), whose Qt 6.7 condition this pull request removes.
- **Lands before:** plan 3. The order is 1, 2, this pull request, 3 to 9, the model-cost round, then 10; plan 10 needs
  the floor for `QRhiWidget`.
- **Branch and ledger:** its own branch from `master`, started once plan 2 has landed; one pull request, a slice of one
  commit (§4). `LEDGER_qt_floor.md` has a row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing". This
  file states the floor's own: the floor is one number, so every place that names a minimum Qt says 6.8, and every
  guard for an older Qt goes, in the same commit; and an install recipe the documents give is run as written in CI, so
  it cannot rot unseen. Its decisions are numbered QF.D1 on.

## 1. Where `master` stands

### 1.1 The declared minimum

`CMakeLists.txt` asks for Qt 6.2 (`find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Network PrintSupport Multimedia
Widgets Svg Test)`, line 39), and 6.9 when building for WebAssembly (line 37). Two version checks follow, in the
translations block:

- `Qt6_VERSION VERSION_GREATER_EQUAL "6.9.0"` (line 447) adds `MERGE_QT_TRANSLATIONS`, which bakes Qt's own catalogs
  into the application's `.qm` files. It stays: it is about 6.9, above the floor.
- `Qt6_VERSION VERSION_GREATER_EQUAL "6.7.0"` (line 451) sets the `lupdate` options, `-tr-function-alias
  tr+=PANDACEPTION -extensions cpp,h,ui -no-obsolete`. On an older Qt, `update_translations` runs without them and
  drops every string wrapped in `PANDACEPTION()`. Plan 1's translatable-strings target sits behind the same check, as
  `master` sets those options (plan 1's §3.13).

No CI job builds anything older than Ubuntu 24.04's apt Qt, 6.4.2 (§1.6), so nothing checks that 6.2 builds.

### 1.2 The guards a 6.8 floor makes dead

Read on `master`, every Qt version check in `App/` and `Tests/` but the 6.12 one (§1.3):

- `App/Core/Common.h:24`: a `qCDebug` override for Qt before 6.3 (a `for` loop over the category's state); 6.3 and
  later use `QT_MESSAGE_LOGGER_COMMON`.
- `App/Core/ThemeManager.cpp:52` and `:80`: the connection to `QStyleHints::colorSchemeChanged` and the read of
  `colorScheme()`, for Qt 6.5 and later, with a guess from the palette's lightness before 6.5. The header's comments
  say so (`ThemeManager.h:130` and `:133`), and so does the developer guide (`DEVELOPER_GUIDE.md:428`).
- `App/Core/ThemeManager.cpp:27`, `:106` and `:128`: three blocks for 6.8 and later that set the style hints' colour
  scheme before the palette is applied (on Windows it decides what the style's standard palette returns) and clear it
  for the System theme so the platform reads the system's setting again. Each has no `#else`, so the code stays and
  the guard goes.
- `Tests/Unit/Common/TestThemeManager.cpp:332`, `:359` and `:385`: three macOS skips before 6.8, where
  `setColorScheme()` does not exist; and `:407` and `:428`: the two colour-scheme tests skip before 6.5.
- `Tests/Common/TestUtils.h:75`: `QVERIFY_THROWS(exType, ...)`, which calls Qt's `QVERIFY_THROWS_EXCEPTION` on 6.7 and
  later and `QVERIFY_EXCEPTION_THROWN` before. It has 37 uses in 11 test files: `TestICInline`, `TestBewavedDolphinGui`,
  `TestArduinoCodeGenUnit`, `TestDolphinExporter`, `TestDolphinFile`, `TestDolphinModelBuilder`,
  `TestDolphinSerializer`, `TestFileUtils`, `TestSerialization`, `TestCircuitExporter` and `TestExportController`. The
  developer guide tells contributors to use it (`DEVELOPER_GUIDE.md:1343`).
- `CMakeLists.txt:451`: the `lupdate` options' 6.7 check (§1.1).
- `App/UI/LanguageManager.cpp:77`: a comment that Qt's own dialog strings stay English "on Qt 6.2..6.8", where
  `MERGE_QT_TRANSLATIONS` is not available.

### 1.3 The guards that stay

They work around releases above the floor: the drag events' 6.12 check (`TestUtils.h:88`) and CMake's 6.9 merge
(§1.1). The theme's comments on Qt 6.9.3's `requestColorScheme()` and the fix 6.10.0 made are about those releases,
not about a guard, and stay as they are.

### 1.4 The documents

Each says Qt 6.2: `README.md:26` and its Spanish and Brazilian Portuguese versions (`README_es.md`, `README_pt_BR.md`,
the same line); `BUILD.md:14` and `:16` and their two translations; `DEVELOPER_GUIDE.md:79` (the technology table),
`:109` (the dev container's Qt, which is in fact 6.9.3, installed by aqt in its `Dockerfile`), `:141` (Windows) and
`:1732` (the troubleshooting table); and `.devcontainer/README.md:25` (also the container's Qt). `BUILD.md` tells
users of "Arch Linux, Gentoo, Manjaro, Debian, etc." to install Qt from their repositories, and its apt line, under
"Debian/Ubuntu", is the one `CONTRIBUTING.md:88` gives as an "example for Ubuntu" and the developer guide repeats
under "Ubuntu/Debian".

### 1.5 The aqt recipe

`BUILD.md`'s "Others" section installs Qt 6.9.3 with aqtinstall on a Debian-based distribution:

```bash
sudo apt-get update
sudo apt-get install libgl1-mesa-dev libxcb-xinerama0 libpulse-dev git python3 python3-pip python3-venv -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt linux desktop 6.9.3 --outputdir ~/Qt
PATH=~/Qt/6.9.3/gcc_64/bin:$PATH
export PATH
```

Run on the branch in a bare Ubuntu 24.04 container, it cannot build: it installs no Qt Multimedia, which
`find_package` requires, so configuring fails; it installs no compiler; and where no desktop brought it, Qt Gui's link
fails for want of `libxkbcommon.so.0`. With the floor it becomes the way users of Ubuntu 22.04 and 24.04 and Debian 12
build, so it has to work as written.

### 1.6 CI

- `build.yml`: the matrix crosses `ubuntu-24.04` and `windows-2025-vs2026` with Qt 6.8.3 and 6.9.3, and adds two Linux
  6.9.3 builds (without unity and precompiled headers; one unity batch of 1,000), `macos-15` with 6.8.3, `macos-26`,
  `ubuntu-24.04-arm` and `windows-11-arm` with 6.9.3; Qt comes from `jurplel/install-qt-action` with the modules
  `qtmultimedia qtimageformats`. The i386 job builds in a Debian container with its Qt, 6.8.2. CodeQL, coverage, the
  sanitizers, the translation workflow, the WebAssembly build and the release all use 6.9.3. So the floor is built and
  tested on Linux, Windows and macOS (6.8.3) and on i386 (6.8.2) already.
- `fresh-deps.yml` builds from clean distribution images with what the documents say, weekly, on demand, and on every
  pull request that changes the build documents, `CONTRIBUTING.md`, the developer guide, `CMakeLists.txt`,
  `CMakePresets.json` or itself: its `distro` job runs `BUILD.md`'s apt line on `ubuntu:24.04` and `debian:trixie`,
  and its `minimum-cmake` job builds on `ubuntu:24.04`'s apt Qt with CMake pinned to 3.27. Ubuntu 24.04's apt Qt is
  6.4.2, so with the floor both of its runs there stop at `find_package`. Nothing runs the aqt recipe.

### 1.7 Distributions and Qt's support

No maintained distribution ships Qt 6.7: Fedora 40 had 6.7.2 and reached its end of life in May 2025, and Alpine went
from 6.6.3 (3.20) to 6.8.2 (3.21). Qt 6.8 or later ships in Debian 13 (6.8.2), Ubuntu 25.04 and later, Fedora, Alpine
3.21 and later, and Arch. Ubuntu 22.04, Ubuntu 24.04 (6.4.2) and Debian 12 (6.4.2) ship an older Qt; their users
install Qt with the aqt recipe, as they would under a 6.7 floor. Qt's standard support for 6.7 ended on 2025-03-26;
6.8 is a long-term-support release, supported until 2029-10-08. The released AppImage, ZIP and DMG bundle their Qt
(6.9.3), so users of the releases are not affected.

## 2. What the pull request changes

The branch raised the floor to 6.7 in one commit (`c4b545131`), and ran each CI job's steps in local containers; the
recipe, the documents' structure and the fresh distro jobs below are its work, with 6.8 for 6.7 and the 6.8 guards
added to its dead ones. It added a 6.7.3 CI build and needed two Qt 6.7.3 fixes and blur masks for it, which a 6.8
floor does not.

### 2.1 CMake

`find_package` asks for 6.8; WebAssembly keeps 6.9. The `lupdate` options and plan 1's translatable-strings target no
longer wait for 6.7: the options pass straight to `qt_add_translations`, and `qt_add_lupdate` is defined whenever the
linguist tools are found.

### 2.2 The dead guards

- `Common.h` keeps only the 6.3 and later `qCDebug`.
- `ThemeManager` always connects to `colorSchemeChanged` and reads `colorScheme()`; the palette-lightness guess goes,
  and the header's and the developer guide's sentences about it. Its three 6.8 blocks run unconditionally.
- The two colour-scheme tests and the three macOS palette tests no longer skip.
- `QVERIFY_THROWS` goes from `TestUtils.h`; its 37 uses call `QVERIFY_THROWS_EXCEPTION(exType, expression)` directly,
  and a test file that included `TestUtils.h` only for it drops the include (four did on the branch:
  `TestDolphinExporter`, `TestDolphinModelBuilder`, `TestFileUtils` and `TestCircuitExporter`; QF.6 checks it again on
  `master`). The developer guide's testing section names `QVERIFY_THROWS_EXCEPTION`.
- `LanguageManager.cpp`'s comment says "on Qt 6.8".

### 2.3 The documents

- Qt 6.8 in every place §1.4 names, in the three languages; the dev container's Qt named as 6.9.3, what it installs.
- `BUILD.md`: Qt 6.8 or later is in the repositories of "Arch Linux, Gentoo, Manjaro, Debian 13+, Ubuntu 25.04+";
  Debian 12 and Ubuntu 22.04 and 24.04 ship an older Qt and use the aqt recipe; the apt section is headed "Debian 13+
  and Ubuntu 25.04+". The developer guide's apt section takes the same heading and the same sentence, and
  `CONTRIBUTING.md`'s comment names the same releases and points older ones to `BUILD.md`.
- Release notes are written at the release, not in this pull request; the `.ts` catalogs are untouched, since no
  translatable string changes.

### 2.4 The aqt recipe

```bash
sudo apt-get update
sudo apt-get install build-essential libgl1-mesa-dev libpulse-dev git python3 python3-pip python3-venv \
    libxcb-cursor0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libxkbcommon-x11-0 \
    libxrandr2 libwayland-cursor0 libwayland-egl1 -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt linux desktop 6.9.3 -m qtmultimedia qtimageformats --outputdir ~/Qt
PATH=~/Qt/6.9.3/gcc_64/bin:$PATH
export PATH
```

It gains a compiler (`build-essential`), Qt Multimedia and Qt Image Formats (as CI installs them), and the libraries
that Qt's X11 and Wayland platform plugins and its FFmpeg multimedia backend load at run time, found with `ldd` on the
plugins; `libxkbcommon-x11-0` brings the `libxkbcommon.so.0` Qt Gui links. `libxcb-xinerama0` goes: no Qt 6 library
links it. A sentence under the recipe says what the run-time libraries are for, and that a desktop install usually has
them.

### 2.5 CI

- `build.yml`: no new build. A comment on the matrix says that the 6.8.3 entries, and the i386 job's Debian 6.8.2,
  build and test the floor `find_package` declares, so a change that drops a 6.8 entry keeps one.
- `fresh-deps.yml`: the `distro` job runs the apt line on `debian:trixie` and `ubuntu:26.04` (Qt 6.10.2 on the
  branch), images whose Qt meets the floor. A new `aqt` job runs the recipe verbatim on `ubuntu:24.04`, building in
  the recipe's own step since its `PATH` line is what makes CMake find that Qt, then checks with `ldd` that the X11
  and Wayland platform plugins and the FFmpeg backend find every library (the build and the offscreen smoke run load
  none of them), then runs the binary headless. The `minimum-cmake` job moves to `debian:trixie`, whose Qt meets the
  floor. The comments at the top of the workflow and of each job say which images use which recipe.

## 3. Pitfalls

- **Neither the build nor the smoke run proves the recipe's libraries.** The platform plugins and the multimedia
  backend load at run time, and the offscreen smoke run loads neither; only `ldd` on the plugins shows a missing
  library.
- **The recipe's `PATH` line lives in one shell.** The CI job builds in the step that ran the recipe; a later step
  would not find that Qt.
- **The theme's unguarded 6.8 blocks run in every test now**, the macOS palette tests included on `macos-15`'s 6.8.3,
  where they already ran; a failure is a `master` defect, a row in plan 2's ledger, never a skip.
- **The translated documents change in their own language**, line for line with the English ones.

## 4. Steps

One pull request of one commit: the floor is one number, and each part needs the others (CI's apt jobs on
`ubuntu:24.04` fail once CMake asks for 6.8; the guards are dead only once it does).

| Row | Step | Slice |
|---|---|---|
| QF.1 | CMake: `find_package` asks for 6.8, WebAssembly keeps 6.9; the `lupdate` options and the translatable-strings target unconditional (§2.1) | 1 |
| QF.2 | The dead guards, with the header's and the developer guide's sentences about them (§2.2) | 1 |
| QF.3 | The documents, in three languages (§2.3) | 1 |
| QF.4 | The aqt recipe (§2.4) | 1 |
| QF.5 | CI: the matrix's comment on the floor, the fresh distro jobs on images whose Qt meets the floor, the aqt job with its `ldd` check (§2.5) | 1 |

The check of §6 is a row too (QF.6).

## 5. Decisions

Each ends with its basis: measured or tested (a run backs it), read (the code, with nothing run), or a ruling (the
user's choice between workable options).

- **QF.D1. Qt 6.8 for the whole project**, WebAssembly keeping 6.9. Users of distributions with an older Qt install Qt
  with the aqt recipe; the releases bundle their own. 6.7, the first with `QRhiWidget`, was weighed and rejected: no
  maintained distribution ships it, so a 6.8 floor loses no user a 6.7 floor keeps; Qt's support for 6.7 ended in
  March 2025, while 6.8 is supported until 2029; and CI's oldest Qt is already 6.8, so 6.7 would have needed a CI
  build of its own, two Qt 6.7.3 fixes before the floor, and blur masks in plan 3's body goldens. Basis: read (the
  distributions' Qt and Qt's support dates, §1.7); the user's rulings of 2026-10-14 (one floor for the whole project,
  not one for GPU builds only) and 2026-10-21 (6.8: "go with 6.8").
- **QF.D2. Right after plan 2, before plan 3**, with a plan file and a ledger of its own. Basis: the user's choice,
  2026-10-21, once the plans were set to run one at a time.
- **QF.D3. The aqt recipe is the documented way for older distributions, and CI runs it as written**, with an `ldd`
  check of the plugins it does not load. Basis: tested on the branch in a bare Ubuntu 24.04 container: the recipe as
  on `master` could not configure, had no compiler and lacked a library Qt Gui links.
- **QF.D4. The fresh distro jobs move to images whose Qt meets the floor**: the apt line on Debian trixie and Ubuntu
  26.04, the minimum CMake on Debian trixie. Basis: read (Ubuntu 24.04's apt Qt is 6.4.2); tested on the branch in
  local containers.
- **QF.D5. No new CI build: the existing 6.8 jobs test the floor.** CI's 6.8.3 builds on Linux, Windows and macOS and
  the i386 job's 6.8.2 are the floor's; a comment on the matrix says so. Basis: read on `master` (§1.6).

## 6. Claims to check and open questions

- **Which test files include `TestUtils.h` only for the wrapper** (QF.6): four on the branch; on `master`, each of the
  11 files is compiled alone without the include, and only those that still compile drop it.

## 7. Risks

| Risk | Mitigation |
|---|---|
| A user of Ubuntu 22.04 or 24.04, or Debian 12, can no longer build with the apt line | `BUILD.md` names those releases and sends them to the aqt recipe, which CI runs as written |
| A user on a distribution with Qt 6.7 can no longer build from source | None is maintained (§1.7); the aqt recipe works there too |
| The recipe rots: a package renamed or dropped | The fresh distro workflow runs it weekly and on every pull request that changes the documents |
| A later change drops CI's last 6.8 build, leaving the floor untested | The matrix's comment names the floor's entries (§2.5) |

## 8. What other plans take from it

- **Plans 3 to 10:** a floor CI already tests, so no plan meets a Qt older than 6.8: the blur filters render alike on
  every supported Qt, and plan 3's body goldens need no masks for an older one.
- **Plan 10:** `QRhiWidget` on every build. `QRhi` is used through `Qt::GuiPrivate` and `<rhi/qrhi.h>`; its private
  headers come in `qt6-base-private-dev` (Debian, Ubuntu) and `qt6-qtbase-private-devel` (Fedora), and aqt and Arch's
  `qt6-base` include them, so the apt lines that gain the private headers are plan 10's change, when it first uses
  them.

## 9. Evidence

A hash is the `gpu-canvas` branch's, kept read-only.

| What | Where |
|---|---|
| The floor raised to 6.7: CMake, the dead guards, the documents, the aqt recipe, CI | `gpu-canvas`: `c4b545131` |
| Qt's release dates and support periods | Qt's documentation, "Qt Releases" (`doc.qt.io/qt-6/qt-releases.html`) |
