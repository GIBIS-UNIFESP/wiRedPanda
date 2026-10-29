# Building

## On Windows

Install Qt 6.12.0 with the [Qt online installer](https://www.qt.io/download), selecting these components:
![qt6](https://github.com/user-attachments/assets/aedc8749-8b5c-4f4d-9c65-651b703dccea)

Then open CMakeLists.txt in Qt Creator and press Ctrl+R to build and run.

## On Linux & macOS

### Dependencies

Qt 6.8.0+ with the Multimedia and Svg modules is needed for building. The recommended Qt is 6.12.0, the version CI
tests and the releases bundle, installed with [aqtinstall](https://github.com/miurahr/aqtinstall), which gives the same
Qt whatever your OS release.

* Debian and Ubuntu

```bash
sudo apt-get update
sudo apt-get install build-essential libgl1-mesa-dev libpulse-dev git python3 python3-pip python3-venv \
    libxcb-cursor0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libxkbcommon-x11-0 \
    libxrandr2 libwayland-cursor0 libwayland-egl1 -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt linux desktop 6.12.0 -m qtmultimedia qtimageformats --outputdir ~/Qt
PATH=~/Qt/6.12.0/gcc_64/bin:$PATH
export PATH
```

The `libxcb-*`, `libxkbcommon-x11-0`, `libxrandr2` and `libwayland-*` packages are what Qt's X11 and Wayland platform plugins
and its multimedia backend load at run time; a desktop install usually has them already. On other distributions,
install their equivalents with your package manager, then run the last five lines.

* macOS (with the Xcode command line tools: `xcode-select --install`)

```bash
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt mac desktop 6.12.0 -m qtmultimedia qtimageformats --outputdir ~/Qt
PATH=~/Qt/6.12.0/macos/bin:$PATH
export PATH
```

* From your distribution's packages

If your distribution ships Qt 6.8 or later (Debian 13, Ubuntu 26.04, Fedora, Arch Linux), its packages work too.
Debian 12 and Ubuntu 22.04 and 24.04 ship an older Qt.

```bash
# Debian 13, Ubuntu 26.04
sudo apt install build-essential qt6-base-dev qt6-multimedia-dev qt6-svg-dev
# Arch Linux-based
sudo pacman -S base-devel qt6-base qt6-multimedia qt6-svg
# macOS, with Homebrew
brew install qt6
```

Qt can also be installed from the [Qt website](https://www.qt.io/download) or
[built from source](https://wiki.qt.io/Building_Qt_6_from_Git).

### Build process

**Requirements**: Install Ninja build system:

* Windows: `choco install ninja` or `scoop install ninja`
* Linux: `apt install ninja-build` or `yum install ninja-build`
* macOS: `brew install ninja`

**Build commands (all platforms):**

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda
cd wiredpanda
cmake --preset release
cmake --build --preset release
```

This process could take a while. Once concluded, the binary will be located at `wiredpanda/build/wiredpanda`, on Linux, and at `wiredpanda/build/wiredpanda.app/Contents/MacOS/wiredpanda` on macOS.

## Licensing

wiRedPanda is licensed under the [GNU General Public License, Version 3.0](http://www.gnu.org/licenses/).

See [`LICENSE`](LICENSE) for the full license text.

```text
Copyright (C) 2026 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres and Vinícius Miguel.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
```
