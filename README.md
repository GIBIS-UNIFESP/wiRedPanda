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
Compiled binaries for Windows and Ubuntu are availble [here](http://gibis-unifesp.github.io/wiRedPanda/downloads/).

## Building WiRED Panda on your computer:
 To compile this software you will need the [Qt5.5 Development Environment ](http://www.qt.io/download-open-source/) and the [source code](https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/v1.9-beta.tar.gz) of the software.
 
 To compile and install this software on linux, please use the following commands:


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
  
    Copyright (C) 2015 - Davi Morales, Fábio Cappabianco, Lucas Lellis and Rodrigo Torres.
    
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
