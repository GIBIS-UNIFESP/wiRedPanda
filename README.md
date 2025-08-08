_Leia-me em [português](README_pt_BR.md). Léame en [español](README_es.md)._

# wiRedPanda ![GitHub All Releases](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square) [![codecov](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda/branch/master/graph/badge.svg?token=5YBYB4J705)](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda) <a href="https://hosted.weblate.org/engage/wiredpanda/"><img src="https://hosted.weblate.org/widget/wiredpanda/svg-badge.svg" alt="Translation status" /></a>


wiRedPanda is a free software designed to help students learn about logic circuits and simulate them in an easy and friendly way.

The main features of the software are:
  - Works on Windows, macOS and Linux;
  - Real time logic simulation;
  - User-friendly interface;
  - It's intuitive and easy to use;
  - Export your work as an image or a PDF.

![Main screen](https://gibis-unifesp.github.io/wiRedPanda/demo.gif)

## Downloads
Compiled binaries for Windows, Linux and macOS are available [here](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Building

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda
cd wiredpanda
cmake -B build
cmake --build build --config Release
```

This process could take a while. Once concluded, the binary will be located at `wiredpanda/build/wiredpanda`, on Linux, and at `wiredpanda/build/wiredpanda.app/Contents/MacOS/wiredpanda` on macOS.

For detailed build instructions, check the [BUILD.md](BUILD.md) or visit the [wiki](https://github.com/GIBIS-UNIFESP/wiRedPanda/wiki/How-to-setup-environment).

## Translation

You can help translate wiRedPanda into other languages. We use the [Weblate](https://hosted.weblate.org/projects/wiredpanda/wiredpanda) platform to manage translations.

Alternatively, you can also contribute by forking this repository and adding translations manually.

The translations are located in the [app/resources/translations](https://github.com/GIBIS-UNIFESP/wiRedPanda/tree/master/app/resources/translations) directory.

<a href="https://hosted.weblate.org/engage/wiredpanda/">
<img src="https://hosted.weblate.org/widget/wiredpanda/wiredpanda/multi-auto.svg" alt="Translation status" />
</a>

## Stars

<a href="https://www.star-history.com/#GIBIS-UNIFESP/wiRedPanda&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date" />
 </picture>
</a>

[![Packaging status](https://repology.org/badge/vertical-allrepos/wiredpanda.svg)](https://repology.org/project/wiredpanda/versions)

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
