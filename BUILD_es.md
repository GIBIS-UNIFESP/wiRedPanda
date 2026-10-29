# Compilando

## En Windows

Instale Qt 6.12.0 con el [instalador en línea de Qt](https://www.qt.io/download), seleccionando en Qt 6.12.0 la
compilación MSVC 2022 64-bit y, en sus Additional Libraries, Qt Multimedia y Qt Image Formats.

Luego abra CMakeLists.txt en Qt Creator y presione Ctrl+R para compilar y ejecutar.

## En Linux y macOS

### Dependencias

Se necesita Qt 6.8.0+ con los módulos Multimedia y Svg para compilar. El Qt recomendado es 6.12.0, la versión que
prueba la CI y que incluyen las versiones publicadas, instalado con [aqtinstall](https://github.com/miurahr/aqtinstall),
que da el mismo Qt sea cual sea la versión de su sistema operativo.

* Debian y Ubuntu

```bash
sudo apt-get update
sudo apt-get install build-essential libgl1-mesa-dev libpulse-dev git python3 python3-pip python3-venv \
    libxcb-cursor0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libxkbcommon-x11-0 \
    libxrandr2 libwayland-cursor0 libwayland-egl1 -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt linux desktop 6.12.0 -m qtmultimedia qtimageformats --outputdir ~/Qt
PATH=~/Qt/6.12.0/gcc_64/bin:$PATH
export PATH
```

Los paquetes `libxcb-*`, `libxkbcommon-x11-0`, `libxrandr2` y `libwayland-*` son los que cargan en tiempo de ejecución
los complementos de plataforma X11 y Wayland de Qt y su backend multimedia; una instalación de escritorio normalmente ya los tiene.
En otras distribuciones, instale sus equivalentes con su gestor de paquetes y luego ejecute las últimas cinco líneas.

* macOS (con las herramientas de línea de comandos de Xcode: `xcode-select --install`)

```bash
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt mac desktop 6.12.0 -m qtmultimedia qtimageformats --outputdir ~/Qt
PATH=~/Qt/6.12.0/macos/bin:$PATH
export PATH
```

* Desde los paquetes de su distribución

Si su distribución incluye Qt 6.8 o posterior (Debian 13, Ubuntu 26.04, Fedora, Arch Linux), sus paquetes también
funcionan. Debian 12 y Ubuntu 22.04 y 24.04 incluyen un Qt más antiguo.

```bash
# Debian 13, Ubuntu 26.04
sudo apt install build-essential qt6-base-dev qt6-multimedia-dev qt6-svg-dev
# Basado en Arch Linux
sudo pacman -S base-devel qt6-base qt6-multimedia qt6-svg
# macOS, con Homebrew
brew install qt6
```

Qt también se puede instalar desde el [sitio web de Qt](https://www.qt.io/download) o
[construir desde la fuente](https://wiki.qt.io/Building_Qt_6_from_Git).

### Proceso de compilación

**Requisitos**: Instale el sistema de compilación Ninja:

* Windows: `choco install ninja` o `scoop install ninja`
* Linux: `apt install ninja-build` o `yum install ninja-build`
* macOS: `brew install ninja`

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
Copyright (C) 2026 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres y Vinícius Miguel.

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
