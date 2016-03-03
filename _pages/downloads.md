---
layout: inner
title: Downloads
permalink: /downloads/
---
To run WiRed Panda on your platform you can:

### Windows:
 Extract the [portable version](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.9-beta/WiredPanda_1_9_Windows_Portable_x86_64.zip) or download the [windows installer](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.9-beta/WiredPanda_1_9_Windows_Installer_x86_64.exe) of the [latest release](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/v1.9-beta) of WiRed Panda.
  
### Linux:
 Try the [portable version](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/v1.8-beta/WiredPanda_Portable_Ubuntu1404_x64.tar.gz) (Compiled on Ubuntu 14.04) of the [latest release](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/v1.9-beta) of WiRed Panda, or else download the [source code](https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/v1.9-beta.tar.gz) and build on you computer.

### Building WiRED Panda on your computer:

 To compile and install this software on a gnome-shell based linux distro with the Qt5.5 development environment, please use the following commands:


```sh
git clone https://github.com/GIBIS-UNIFESP/wiRedPanda/
mkdir wiRedPanda/build
cd wiRedPanda/build
qmake ../WPanda.pro
sudo make install
```
