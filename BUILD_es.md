# Compilando

## En Windows

Para Qt5 seleccione esto:
![qt5](https://github.com/user-attachments/assets/e6bfca48-7b4b-444f-9ad3-76c157a9036a)

O para Qt6 selecciona estos:
![qt6](https://github.com/user-attachments/assets/aedc8749-8b5c-4f4d-9c65-651b703dccea)

Luego abra CMakeLists.txt en Qt Creator y presione Ctrl+R para compilar y ejecutar.

## En Linux y macOS

### Dependencias

Se necesita Qt 5.15.0+ o 6.2.0+ y el módulo QtMultimedia para compilar.

En distribuciones como Arch Linux, Gentoo, Manjaro, Debian, etc., Qt 5.15+ se puede instalar desde los repositorios estándar.

* Debian/Ubuntu

```bash
sudo apt install build-essential qtbase5-dev qtmultimedia5-dev libqt5svg5-dev
```

* Basado en Arch Linux

```bash
sudo pacman -S base-devel qt5-base qt5-multimedia qt5-svg
```

* macOS

```bash
brew install qt5
```

* Otros

Qt5 se puede instalar directamente desde el
[sitio web de Qt](https://www.qt.io/download), desde instaladores no oficiales
como [aqtinstall](https://github.com/miurahr/aqtinstall), desde repositorios
mantenidos por la comunidad o
[construidos desde la fuente](https://wiki.qt.io/Building_Qt_5_from_Git).

Así es como se podría usar aqtinstall para instalar Qt 5.15.2 en una
distribución basada en Debian.

```bash
sudo apt-get update
sudo apt-get install libgl1-mesa-dev libxcb-xinerama0 libpulse-dev git python3 python3-pip python3-venv -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt linux desktop 5.15.2 --outputdir ~/Qt
PATH=~/Qt/5.15.2/gcc_64/bin:$PATH
export PATH
```

### Proceso de compilación

**Requisitos**: Instale el sistema de compilación Ninja:
- Windows: `choco install ninja` o `scoop install ninja`
- Linux: `apt install ninja-build` o `yum install ninja-build`
- macOS: `brew install ninja`

**Comandos de compilación (todas las plataformas):**

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda
cd wiredpanda
cmake --preset release
cmake --build --preset release
```

Este proceso podría llevar un tiempo. Una vez concluido, el binario se ubicará en `wiredpanda/build/wiredpanda`, en Linux, y en `wiredpanda/build/wiredpanda.app/Contents/MacOS/wiredpanda` en macOS.

## Licencia

wiRedPanda tiene licencia bajo la [Licencia pública general de GNU, versión 3.0](http://www.gnu.org/licenses/).

Consulte [`LICENSE`](LICENSE) para obtener el texto completo de la licencia.

```text
Copyright (C) 2025 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres y Vinícius Miguel.

Este programa es software libre: puedes redistribuirlo y/o modificar
bajo los términos de la Licencia Pública General GNU publicada por
la Free Software Foundation, ya sea la versión 3 de la Licencia, o
(a su elección) cualquier versión posterior.

Este programa se distribuye con la esperanza de que sea útil,
pero SIN NINGUNA GARANTIA; sin siquiera la garantía implícita de
COMERCIABILIDAD o IDONEIDAD PARA UN FIN DETERMINADO. Ver el
Licencia Pública General GNU para más detalles.

Debería haber recibido una copia de la Licencia Pública General GNU
junto con este programa. Si no, consulte http://www.gnu.org/licenses/.
```
