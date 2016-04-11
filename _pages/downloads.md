---
layout: inner
title: Downloads
permalink: /downloads/
---
The binaries of the [latest release](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/2.1-rc) of wiRED Panda are availble in the following platforms:

- [Windows x86_64 Portable](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/2.1-rc/WiredPanda_2_1_Windows_Portable_x86_64.zip)
- [Windows x86_64 Installer](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/2.1-rc/WiredPanda_2_1_Windows_Installer_x86_64.exe)
- [Ubuntu 12.04+ Installer](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/2.1-rc/WiredPanda_2_1_Ubuntu_Installer_x64.deb)
- [Linux Portable](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/2.1-rc/WiredPanda_2_1_Linux_Portable_x64.tar.gz) (Tested on Ubuntu 12.04+ and fedora 23)

wiRed Panda is also compatible with OSX and many linux distributions, but we don't have compiled versions availble for download. We are planning to release the OSX version in the following months, for now the software can be compiled on your own computer. 

### Building WiRED Panda on your computer:
 To compile this software you will need the [Qt5.5 Development Environment ](http://www.qt.io/download-open-source/) and the [source code](https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/2.1-rc.tar.gz) of the software.
 
 To compile and install this software on linux, please use the following commands:


```sh
git clone https://github.com/GIBIS-UNIFESP/wiRedPanda/
mkdir wiRedPanda/build
cd wiRedPanda/build
qmake ../WPanda.pro
sudo make install
```
