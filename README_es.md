# wiRedPanda ![Todos los lanzamientos de GitHub](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![Lanzamiento de GitHub (último por fecha)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square)


wiRedPanda es un software gratuito diseñado para ayudar a los estudiantes a aprender sobre circuitos lógicos y simularlos de una manera fácil y amigable.

Las principales características del software son:
  - Funciona en Windows, macOS y Linux;
  - Simulación lógica en tiempo real;
  - Interfaz amigable;
  - Es intuitivo y fácil de usar;
  - Exporta tu trabajo como una imagen o un PDF.

_Leia-me em [português](README_pt_BR.md). Léame en [english](README.md)._

![Pantalla principal](https://gibis-unifesp.github.io/wiRedPanda/images/ffms.gif)

## Descargas
Los binarios compilados para Windows y Linux están disponibles [aquí](http://gibis-unifesp.github.io/wiRedPanda/downloads/).

Los binarios enlazados dinámicamente para macOS están disponibles [aquí](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Compilando

### En Linux y macOS

#### Dependencias

Se necesita Qt 5.7.0+ o 6.2.0+ para compilar, así como QtCharts y QtMultimedia.

En distribuciones como Arch Linux, Gentoo, Manjaro, Debian Testing, etc., Qt 5.7+ se puede instalar desde los repositorios estándar.

* Debian Testing

```bash
sudo apt install qtbase5-dev qt5-make qtbase5-dev-tools qtchooser libqt5charts5-dev libqt5multimedia5-dev
```

* Basado en Arch Linux

```bash
sudo pacman -S qt5-base qt5-charts qt5-multimedia
```

* macOS

```bash
brew install qt5
```

* Otros

Qt5 se puede instalar directamente desde el [sitio web de Qt] (https://www.qt.io/download), desde instaladores no oficiales como [aqtinstall] (https://github.com/miurahr/aqtinstall), desde repositorios mantenidos por la comunidad o [construidos desde la fuente] (https://wiki.qt.io/Building_Qt_5_from_Git).

Así es como se podría usar aqtinstall para instalar Qt 5.15.1 (se necesita Python 3) junto con los complementos necesarios en una distribución basada en Debian.

```bash
sudo apt-get update
sudo apt-get install build-essential libgl1-mesa-dev libxkbcommon-x11-0 libpulse-dev -y
python3 -m pip install setuptools wheel
python3 -m pip install py7zr==0.10.1
python3 -m pip install aqtinstall==0.9.7
python3 -m pip install importlib-metadata==2.0.0
python3 -m aqt install 5.15.1 linux desktop -m qtcharts qtmultimedia -O ~/Qt
export Qt5_Dir=~/Qt5/5.15.1
export Qt5_DIR=~/Qt5/5.15.1
export QT_PLUGIN_PATH=~/Qt/5.15.1/gcc_64/plugins
PATH=~/Qt/5.15.1/gcc_64/bin:$PATH
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

Este proceso podría llevar un tiempo. Una vez concluido, el binario se ubicará en `wiredpanda/build/app/wpanda`, en Linux, y en `wiredpanda/build/app/wpanda.app/Contents/MacOS/wpanda` en macOS.

## Licencia

wiRedPanda tiene licencia bajo la [Licencia pública general de GNU, versión 3.0] (http://www.gnu.org/licenses/).

Consulte [`LICENCIA`](LICENCIA) para obtener el texto completo de la licencia.
  
    Copyright (C) 2022 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres y Vinícius Miguel.
    
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