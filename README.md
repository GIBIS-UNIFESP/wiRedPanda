# WiRedPanda ![GitHub All Releases](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square)


WiRedPanda is a free software designed to help students learn about logic circuits and simulate them in an easy and friendly way.

The main features of the software are:
  - Works on Windows, macOS and Linux;
  - Real time logic simulation;
  - User-friendly interface;
  - It's intuitive and easy to use;
  - Export your work as an image or a PDF.

_Leia-me em [português](README_pt_BR.md). Léame en [español](README_es.md)._

![Main screen](https://gibis-unifesp.github.io/wiRedPanda/images/ffms.gif)

## Downloads
Compiled binaries for Windows and Linux are available [here](http://gibis-unifesp.github.io/wiRedPanda/downloads/).

Dynamically-linked binaries for macOS are available [here](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Building

### On Linux & macOS

#### Dependencies 

Qt 5.7.0+ or 6.2.0+ is needed for building, as well as QtCharts and QtMultimedia. 

On distros such as Arch Linux, Gentoo, Manjaro, Debian Testing, etc., Qt 5.7+ can be installed from the standard repos.

* Debian Testing

```bash
sudo apt install qtbase5-dev qt5-make qtbase5-dev-tools qtchooser libqt5charts5-dev libqt5multimedia5-dev
```

* Arch Linux-based

```bash
sudo pacman -S qt5-base qt5-charts qt5-multimedia
```

* macOS

```bash
brew install qt5
```

* Others

Qt5 may be installed directly from the [Qt website](https://www.qt.io/download), from unofficial installers such as [aqtinstall](https://github.com/miurahr/aqtinstall), from community-maintained repositories or [built from source](https://wiki.qt.io/Building_Qt_5_from_Git).

Here's how one could use aqtinstall to install Qt 5.15.1 (Python 3 needed) alongside with the necessary plugins on a Debian-based distro.

```bash
sudo apt-get update
sudo apt-get install build-essential libgl1-mesa-dev libxkbcommon-x11-0 libpulse-dev -y
python3 -m pip install setuptools wheel
python3 -m pip install py7zr==0.10.1
python3 -m pip install aqtinstall==0.9.7
python3 -m pip install importlib-metadata==2.0.0
python3 -m aqt install 5.15.1 linux desktop -m qtcharts qtmultimedia -O ~/Qt
export Qt5_Dir=~/Qt5/5.15.1
export Qt5_DIR=~/Qt5/5.15.1
export QT_PLUGIN_PATH=~/Qt/5.15.1/gcc_64/plugins
PATH=~/Qt/5.15.1/gcc_64/bin:$PATH
export PATH
```

#### Build process

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
mkdir wiredpanda/build
cd wiredpanda/build
qmake ../WPanda.pro
make -j
```

This process could take a while. Once concluded, the binary will be located at `wiredpanda/build/app/wpanda`, on Linux, and at `wiredpanda/build/app/wpanda.app/Contents/MacOS/wpanda` on macOS.

## Licensing

WiRedPanda is licensed under the [GNU General Public License, Version 3.0](http://www.gnu.org/licenses/).

See [`LICENSE`](LICENSE) for the full license text.
  
    Copyright (C) 2022 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres and Vinícius Miguel.
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.