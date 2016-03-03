# WiRed Panda

[![Build Status](https://travis-ci.org/GIBIS-UNIFESP/wiRedPanda.svg?branch=master)](https://travis-ci.org/GIBIS-UNIFESP/wiRedPanda)

WiRed Panda is a free software designed to help students to learn about logic circuits and simulate them in an easy and friendly way.

The main features of the software are:
  - Works on Windows, OSX and Linux (Thanks Qt :D)
  - Real time logic simulation.
  - User-friendly interface.
  - It's intuitive and easy to use.
  - [**new**] Export your projects to Arduino!

![Alt text](images/wpanda.png?raw=true "WiRed Panda Screenshot")

## Downloads
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

##License
This software is licensed under the [GNU General Public License, Version 3.0](http://www.gnu.org/licenses/).

See [`LICENSE`](LICENSE) for full of the license text.
  
    Copyright (C) 2015 - Davi Morales, Fábio Cappabianco, Héctor Castelli, Lucas Lellis and Rodrigo Torres.
    
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
