---
layout: inner
title: Downloads
permalink: /downloads/
---
To install and run WiRed Panda on your platform you can:

### Windows:
 Extract the [portable version](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.8-beta/wpanda_portable_32.zip) or download the [windows installer](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.8-beta/wpanda_32.exe) of the [latest release](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/v1.8-beta) of WiRed Panda.
 
### Linux:
 Try to download and execute the [portable version](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.8-beta/wpanda_linux64_portable.tar.gz) of the [latest release](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/v1.8-beta) of WiRed Panda, or else download the source code and build on you computer.

### Build on your computer:

 To compile and install this software on a gnome-shell based linux distro with the Qt5.5.1 development environment, please use the following commands:
 
```sh
git clone https://github.com/GIBIS-UNIFESP/wiRedPanda/
mkdir wiRedPanda/build
cd wiRedPanda/build
qmake ../WPanda.pro
make install
```
