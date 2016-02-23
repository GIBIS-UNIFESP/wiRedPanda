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

##Install
To install and run WiRed Panda on your platform you can:

### Windows:
 Download the [windows installer] (https://github.com/GIBIS-UNIFESP/wiRedPanda/raw/master/installer/wpanda17_32.exe) of the latest version of WiRed Panda.
 
### Linux:
 To compile and install this software on a gnome-shell based linux distro, please use the following commands:
 
```sh
git clone https://github.com/GIBIS-UNIFESP/wiRedPanda/
mkdir wiRedPanda/build
cd wiRedPanda/build
qmake ../WPanda.pro
make install
```
 
Or else you can clone this repository and build the software using Qt 5.5 or later.


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
