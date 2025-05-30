## Building

### On Windows

For Qt5 select this:
![qt5](https://github.com/user-attachments/assets/e6bfca48-7b4b-444f-9ad3-76c157a9036a)

Or for Qt6 select these:
![qt6](https://github.com/user-attachments/assets/aedc8749-8b5c-4f4d-9c65-651b703dccea)

Then open WPanda.pro in Qt Creator and press Ctrl+R to build and run.


### On Linux & macOS

#### Dependencies 

Qt 5.12.0+ or 6.2.0+ and the QtMultimedia module is needed for building.

On distros such as Arch Linux, Gentoo, Manjaro, Debian, etc., Qt 5.12+ can be installed from the standard repos.

* Debian/Ubuntu

```bash
sudo apt install build-essential qtbase5-dev qtmultimedia5-dev libqt5svg5-dev
```

* Arch Linux-based

```bash
sudo pacman -S base-devel qt5-base qt5-multimedia qt5-svg
```

* macOS

```bash
brew install qt5
```

* Others

Qt5 may be installed directly from the [Qt website](https://www.qt.io/download), from unofficial installers such as [aqtinstall](https://github.com/miurahr/aqtinstall), from community-maintained repositories or [built from source](https://wiki.qt.io/Building_Qt_5_from_Git).

Here's how one could use aqtinstall to install Qt 5.15.2 on a Debian-based distro.

```bash
sudo apt-get update
sudo apt-get install libgl1-mesa-dev libxcb-xinerama0 libpulse-dev git python3 python3-pip python3-venv -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.2.*
~/venv/bin/aqt install-qt linux desktop 5.15.2 --outputdir ~/Qt
PATH=~/Qt/5.15.2/gcc_64/bin:$PATH
export PATH
```

#### Build process

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda
mkdir wiredpanda/build
cd wiredpanda/build
qmake ../WPanda.pro
make -j
```

This process could take a while. Once concluded, the binary will be located at `wiredpanda/build/app/wiredpanda`, on Linux, and at `wiredpanda/build/app/wiredpanda.app/Contents/MacOS/wiredpanda` on macOS.

## Licensing

wiRedPanda is licensed under the [GNU General Public License, Version 3.0](http://www.gnu.org/licenses/).

See [`LICENSE`](LICENSE) for the full license text.
  
    Copyright (C) 2025 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres and Vinícius Miguel.
    
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
