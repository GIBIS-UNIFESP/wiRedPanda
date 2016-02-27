---
layout: inner
title: Downloads
permalink: /downloads/
---
## Install

To install and run WiRed Panda on your platform you can:

### Windows:
 Extract the [portable version](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.8-beta/WiRedPanda_Portable_Windows_x86_64.zip) or download the [windows installer](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.8-beta/WiRedPanda_Installer_Windows_x86_64.exe) of the [latest release](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/v1.8-beta) of WiRed Panda.
 
### Linux:
 Try the [portable version for Fedora 23 x64](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.8-beta/WiredPanda_Portable_Fedora23_x64.gz) of the [latest release](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/v1.8-beta) of WiRed Panda, or else download the [source code](https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/v1.8-beta.tar.gz) and build on you computer.

### Building WiRED Panda on your computer:

 To compile and install this software on a gnome-shell based linux distro with the Qt5.5.1 development environment, please use the following commands:
 
```sh
git clone https://github.com/GIBIS-UNIFESP/wiRedPanda/
mkdir wiRedPanda/build
cd wiRedPanda/build
qmake ../WPanda.pro
sudo make install
```
