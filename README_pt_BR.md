# WiRedPanda ![GitHub All Releases](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square) [![codecov](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda/branch/master/graph/badge.svg?token=5YBYB4J705)](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda)


WiRedPanda é um software gratuito projetado para ajudar os alunos a aprender sobre circuitos lógicos e simulá-los de maneira fácil e amigável.

As principais características do software são:
  - Funciona em Windows, macOS e Linux;
  - Simulação lógica em tempo real;
  - Interface amigável;
  - É intuitivo e fácil de usar;
  - Exporte seu trabalho como imagem ou PDF.

_Read me in [english](README.md). Léame en [español](README_es.md)._

![Tela principal](https://gibis-unifesp.github.io/wiRedPanda/images/ffms.gif)

## Downloads
Binários compilados para Windows e Linux estão disponíveis [aqui](http://gibis-unifesp.github.io/wiRedPanda/downloads/).

Binários vinculados dinamicamente para macOS estão disponíveis [aqui](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Compilação

### No Linux e macOS

#### Dependências

Qt 5.7.0+ ou 6.2.0+ é necessário para compilar, assim como QtCharts e QtMultimedia.

Em distribuições como Arch Linux, Gentoo, Manjaro, Debian Testing, etc., o Qt 5.7+ pode ser instalado a partir dos repositórios padrão.

* Debian Testing

```bash
sudo apt install qtbase5-dev qt5-make qtbase5-dev-tools qtchooser libqt5charts5-dev libqt5multimedia5-dev
```

* Baseado em Arch Linux

```bash
sudo pacman -S qt5-base qt5-charts qt5-multimedia
```

* macOS

```bash
brew install qt5
```

* Outras

Qt5 pode ser instalado diretamente do [site Qt](https://www.qt.io/download), de instaladores não oficiais como [aqtinstall](https://github.com/miurahr/aqtinstall), de repositórios mantidos pela comunidade ou [construídos a partir da fonte](https://wiki.qt.io/Building_Qt_5_from_Git).

Aqui está como alguém pode usar o aqtinstall para instalar o Qt 5.15.1 (Python 3 necessário) junto com os plugins necessários em uma distribuição baseada no Debian.

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

#### Processo de compilação

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
mkdir wiredpanda/build
cd wiredpanda/build
qmake ../WPanda.pro
make -j
```

Este processo pode demorar um pouco. Uma vez concluído, o binário estará localizado em `wiredpanda/build/app/wpanda`, no Linux, e em `wiredpanda/build/app/wpanda.app/Contents/MacOS/wpanda` no macOS.

## Licença

WiRedPanda está licenciado sob a [GNU General Public License, Versão 3.0](http://www.gnu.org/licenses/).

Consulte [`LICENSE`](LICENSE) para obter o texto completo da licença.
  
    Copyright (C) 2022 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres e Vinícius Miguel.
    
    Este programa é um software livre: você pode redistribuí-lo e/ou modificá-lo
    sob os termos da Licença Pública Geral GNU conforme publicada pela 
	Free Software Foundation, seja a versão 3 da Licença, ou
    (a seu critério) qualquer versão posterior.
    
    Este programa é distribuído na esperança de que seja útil,
    mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de
    COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM. Veja a
    Licença Pública Geral GNU para mais detalhes.
    
    Você deve ter recebido uma cópia da Licença Pública Geral GNU
    junto com este programa. Caso contrário, consulte <http://www.gnu.org/licenses/>.