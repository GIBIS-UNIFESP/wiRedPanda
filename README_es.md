# WiRed Panda ![GitHub All Releases](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square)


WiRed Panda es [Software Libre](https://es.wikipedia.org/wiki/Software_libre) diseñado para ayudar a los estudiantes a aprender sobre la lógica detrás de los circuitos eléctricos, y simularlos de una forma fácil e intuitiva.

Las principales características de WiRed Panda son:
- Es multiplataforma, funciona en Windows, MacOs y Linux;
- Simulación de lógica en tiempo real;
- Interfaz amigable para el usuario;
- Es intuitivo y fácil de usar;
- Puede exportar su trabajo como una imagen o un PDF.

_Leia-me em [português](README_pt_BR.md). Read me in [English](README.md)._    

![Pantalla principal](https://user-images.githubusercontent.com/36349314/97934063-532ed000-1d53-11eb-9667-73ea32f456ce.png)

## Descargas
Binarios precompilados para **Windows** y **Linux** están disponibles [aquí](http://gibis-unifesp.github.io/wiRedPanda/downloads/).

Binarios vinculados dinamicamente para **Linux** y **MacOs** están disponibles [aquí](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Compilación

### En Linux y MacOs

#### Dependencias

* Distribuciones basadas en Debian Buster o Ubuntu 18.04
```bash
sudo apt install qt5-default qt5-qmake qtmultimedia5-dev libqt5charts5-dev
```

* Distribuciones basadas en Debian Stretch o Ubuntu 16.04

Algunas dependencias no están disponibles a travez de los repositorios estándar.

```bash
sudo add-apt-repository ppa:beineri/opt-qt-5.10.1-xenial
sudo apt update 
sudo apt install dh-make qt510base qt510charts-no-lgpl qt510multimedia
source /opt/qt510/bin/qt510-env.sh
```

* Ubuntu 14.04+ y Debian Jessie
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

* MacOs

Puede [compilar Qt5 a partir del código fuente](https://doc.qt.io/qt-5/macos-building.html), o puede instalarlo usando [Homebrew](https://brew.sh/).

```bash
brew install qt
```

### Proceso de compilación

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
mkdir wiredpanda/build
cd wiredpanda/build
qmake ../WPanda.pro
make -j$(nproc)
```

Este proceso puede tardar unos momentos. Una vez finalizado el proceso de compilación podrá encontrar el binario en '/wiredpanda/build/app/wpanda' **si esta usando Linux**, y en 'wiredpanda/build/app/wpanda.app' **si esta usando MacOs**.

## Licencia

WiRed Panda esta bajo la 3ra versión de la Licencia Pública General GNU ([GPL3.0](http://www.gnu.org/licenses/)).

Para más información consultar el archivo [`LICENSE`](LICENSE).

    Copyright (C) 2020 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres and Vinícius Miguel.
    
    Este programa es software libre: puedes redistribuirlo y/o modificarlo
    bajo los términos de la Licencia Pública General GNU publicada por la
    Free Software Foundation, ya sea la versión 3 de la licencia,
    o (a su elección) cualquier versión posterior.
    
    Este programa se distribuye con la esperanza de que sea útil, pero
    SIN NINGUNA GARANTÍA; sin siquiera la garantía implícita de COMERCIALIDAD
    o APTITUD PARA UN PROPÓSITO PARTICULAR. Para más detalles consultar la
    Licencia Pública General GNU.
    
    Deberá haber recibido una copia de la Licencia Pública General GNU
    junto con este programa.
    De lo contrario, consulte <http://www.gnu.org/licenses/>
