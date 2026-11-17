# wiRedPanda Packaging

This directory contains packaging definitions and upstream templates for distributing **wiRedPanda** across auto-updating package ecosystems.

## Overview

| Ecosystem | File | Update Mechanism |
| :--- | :--- | :--- |
| **Arch Linux (AUR)** | [`AUR/PKGBUILD`](AUR/PKGBUILD) | Auto-published on GitHub Release via [`.github/workflows/aur-publish.yml`](../.github/workflows/aur-publish.yml) |
| **Flathub (Flatpak)** | [`Flatpak/io.github.gibis_unifesp.wiRedPanda.yml`](Flatpak/io.github.gibis_unifesp.wiRedPanda.yml) | Auto-updated on Flathub via `flatpak-external-data-checker` (`x-checker-data`) |
| **NixOS / Nixpkgs** | [`Nix/package.nix`](Nix/package.nix) | Auto-updated in `NixOS/nixpkgs` (`pkgs/by-name/wi/wiredpanda/package.nix`) via `nix-update-script` (`r-ryantm` / `nixpkgs-update`) |
| **Homebrew Cask (macOS DMG)** | [`Homebrew/Casks/wiredpanda.rb`](Homebrew/Casks/wiredpanda.rb) | Auto-updated in `Homebrew/homebrew-cask` via `livecheck` (`:github_latest` strategy) + `BrewTestBot` |
| **Homebrew Formula (Source)** | [`Homebrew/Formula/wiredpanda.rb`](Homebrew/Formula/wiredpanda.rb) | Auto-updated in `Homebrew/homebrew-core` (or custom tap) via `livecheck` + `BrewTestBot` |

## Offline / Sandboxed Build Flags

All distribution builds that forbid network access during the build step should pass:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DWIREDPANDA_USE_SYSTEM_JSON=ON \
  -DBUILD_TESTING=OFF
```

- `WIREDPANDA_USE_SYSTEM_JSON=ON`: Uses system-installed `nlohmann_json` and `nlohmann_json_schema_validator` instead of `FetchContent`.
- `BUILD_TESTING=OFF`: Skips compiling the `test_wiredpanda` test binary when tests are not being executed (`ON` by default).

## AppStream MetaInfo & Desktop Integration

On Linux (`cmake --install`), CMake automatically installs:
- `share/applications/wiredpanda.desktop`
- `share/metainfo/io.github.gibis_unifesp.wiRedPanda.metainfo.xml`
- `share/mime/packages/wiredpanda.xml`
- `share/icons/hicolor/{26x26,32x32,48x48,64x64,128x128,256x256}/apps/wpanda.png`
- `share/icons/hicolor/scalable/apps/wpanda.svg`
- `share/wiredpanda/Examples/*.panda`

Validate the AppStream MetaInfo file locally with:

```bash
appstreamcli validate --pedantic --explain --no-net App/Resources/Freedesktop/io.github.gibis_unifesp.wiRedPanda.metainfo.xml
```
