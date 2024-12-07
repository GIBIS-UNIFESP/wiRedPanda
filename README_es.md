# WiRedPanda ![Todos los lanzamientos de GitHub](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![Lanzamiento de GitHub (último por fecha)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square) [![codecov](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda/branch/master/graph/badge.svg?token=5YBYB4J705)](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda)


WiRedPanda es un software gratuito diseñado para ayudar a los estudiantes a aprender sobre circuitos lógicos y simularlos de una manera fácil y amigable.

Las principales características del software son:
  - Funciona en Windows, macOS y Linux;
  - Simulación lógica en tiempo real;
  - Interfaz amigable;
  - Es intuitivo y fácil de usar;
  - Exporta tu trabajo como una imagen o un PDF.

_Leia-me em [português](README_pt_BR.md). Léame en [english](README.md)._

![Pantalla principal](https://gibis-unifesp.github.io/wiRedPanda/images/demo.gif)

## Descargas
Los binarios compilados para Windows, Linux y macOS están disponibles [aquí](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Compilando

### En Windows

Para Qt5 seleccione esto:
![qt5](https://github.com/user-attachments/assets/e6bfca48-7b4b-444f-9ad3-76c157a9036a)

O para Qt6 selecciona estos:
![qt6](https://github.com/user-attachments/assets/aedc8749-8b5c-4f4d-9c65-651b703dccea)

Luego abra WPanda.pro en Qt Creator y presione Ctrl+R para compilar y ejecutar.


### En Linux y macOS

#### Dependencias

Se necesita Qt 5.12.0+ o 6.2.0+ y el módulo QtMultimedia para compilar.

En distribuciones como Arch Linux, Gentoo, Manjaro, Debian Testing, etc., Qt 5.12+ se puede instalar desde los repositorios estándar.

* Debian Testing

```bash
sudo apt install qtbase5-dev qt5-make qtbase5-dev-tools qtchooser libqt5multimedia5-dev
```

* Basado en Arch Linux

```bash
sudo pacman -S qt5-base qt5-multimedia
```

* macOS

```bash
brew install qt5
```

* Otros

Qt5 se puede instalar directamente desde el [sitio web de Qt] (https://www.qt.io/download), desde instaladores no oficiales como [aqtinstall] (https://github.com/miurahr/aqtinstall), desde repositorios mantenidos por la comunidad o [construidos desde la fuente] (https://wiki.qt.io/Building_Qt_5_from_Git).

Así es como se podría usar aqtinstall para instalar Qt 5.15.2 en una distribución basada en Debian.

```bash
sudo apt-get update
sudo apt-get install libgl1-mesa-dev libxcb-xinerama0 libpulse-dev git python3 python3-pip python3-venv -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.1.*
~/venv/bin/aqt install-qt linux desktop 5.15.2 --outputdir ~/Qt
PATH=~/Qt/5.15.2/gcc_64/bin:$PATH
export PATH
```

#### Proceso de compilación

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
mkdir wiredpanda/build
cd wiredpanda/build
qmake ../WPanda.pro
make -j
```

Este proceso podría llevar un tiempo. Una vez concluido, el binario se ubicará en `wiredpanda/build/app/wiredpanda`, en Linux, y en `wiredpanda/build/app/wiredpanda.app/Contents/MacOS/wiredpanda` en macOS.

## Licencia

WiRedPanda tiene licencia bajo la [Licencia pública general de GNU, versión 3.0] (http://www.gnu.org/licenses/).

Consulte [`LICENCIA`](LICENCIA) para obtener el texto completo de la licencia.
  
    Copyright (C) 2024 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres y Vinícius Miguel.
    
    Este programa es software libre: puedes redistribuirlo y/o modificar
    bajo los términos de la Licencia Pública General GNU publicada por
    la Free Software Foundation, ya sea la versión 3 de la Licencia, o
    (a su elección) cualquier versión posterior.
    
    Este programa se distribuye con la esperanza de que sea útil,
    pero SIN NINGUNA GARANTIA; sin siquiera la garantía implícita de
    COMERCIABILIDAD o IDONEIDAD PARA UN FIN DETERMINADO. Ver el
    Licencia Pública General GNU para más detalles.
    
    Debería haber recibido una copia de la Licencia Pública General GNU
    junto con este programa. Si no, consulte <http://www.gnu.org/licenses/>.
