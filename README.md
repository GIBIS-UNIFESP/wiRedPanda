# WiRed Panda

[![Build Status](https://travis-ci.org/GIBIS-UNIFESP/wiRedPanda.svg?branch=master)](https://travis-ci.org/GIBIS-UNIFESP/wiRedPanda)

WiRed Panda is a free software designed to help students to learn about logic circuits and simulate them in an easy and friendly way.

The main features of the software are:
  - Works on Windows, macOS and Linux;
  - Real time logic simulation;
  - User-friendly interface;
  - It's intuitive and easy to use;
  - Export your work as an image or a PDF.

_Leia-me em [português](README_pt_BR.md). Léame en [español](README_es.md).

![Main screen](https://user-images.githubusercontent.com/36349314/97934063-532ed000-1d53-11eb-9667-73ea32f456ce.png)

## Downloads

Compiled binaries for Windows and Linux are available [here](http://gibis-unifesp.github.io/wiRedPanda/downloads/).
Dynamically-linked binaries for Linux and macOS are available [here](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Building

### On Linux & macOS

#### Dependencies 

* Debian Buster 10+ or Ubuntu 18.04+ based distros

```bash
sudo apt install qt5-default qt5-qmake qtmultimedia5-dev libqt5charts5-dev
```

* Debian Stretch 8+ or Ubuntu 16.04+ based distros

Some dependencies needed aren't available from the standard repositories.

```bash
sudo add-apt-repository ppa:beineri/opt-qt-5.10.1-xenial
sudo apt update 
sudo apt install dh-make qt510base qt510charts-no-lgpl qt510multimedia
source /opt/qt510/bin/qt510-env.sh
```

* Ubuntu 14.04+ (and equivalently-old Debian) based distros

```bash
sudo add-apt-repository ppa:beineri/opt-qt-5.10.1-trusty
sudo apt update 
sudo apt install dh-make qt510base qt510charts-no-lgpl qt510multimedia
source /opt/qt510/bin/qt510-env.sh
```


* Fedora

```bash
sudo dnf install qt5 qt5-devel qt5-qtmultimedia-devel qt5-charts-devel
```

* macOS

You can either [build Qt5 from source](https://doc.qt.io/qt-5/macos-building.html), or get it from [Homebrew](https://brew.sh/).

```bash
brew install qt
```

#### Build process

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
mkdir wiredpanda/build
cd wiredpanda/build
qmake ../WPanda.pro
make -j$(nproc)
```

This process could take a while. Once concluded, the binary will be located at `wiredpanda/build/app/wpanda`, on Linux, and at `wiredpanda/build/app/wpanda.app/Contents/MacOS/wpanda` on macOS.

## Licensing

WiRed Panda is licensed under the [GNU General Public License, Version 3.0](http://www.gnu.org/licenses/).

See [`LICENSE`](LICENSE) for the full license text.
  
    Copyright (C) 2020 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres and Vinícius Miguel.
    
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

