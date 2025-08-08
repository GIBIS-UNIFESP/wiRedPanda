## Compilação

### No Windows

Para Qt5 selecione isso:
![qt5](https://github.com/user-attachments/assets/e6bfca48-7b4b-444f-9ad3-76c157a9036a)

Ou para Qt6 selecione esses:
![qt6](https://github.com/user-attachments/assets/aedc8749-8b5c-4f4d-9c65-651b703dccea)

Em seguida, abra o CMakeLists.txt no Qt Creator e pressione Ctrl+R para compilar e executar.


### No Linux e macOS

#### Dependências

Qt 5.15.0+ ou 6.2.0+ e o módulo QtMultimedia são necessários para compilar.

Em distribuições como Arch Linux, Gentoo, Manjaro, Debian, etc., o Qt 5.15+ pode ser instalado a partir dos repositórios padrão.

* Debian/Ubuntu

```bash
sudo apt install build-essential qtbase5-dev qtmultimedia5-dev libqt5svg5-dev
```

* Baseado em Arch Linux

```bash
sudo pacman -S base-devel qt5-base qt5-multimedia qt5-svg
```

* macOS

```bash
brew install qt5
```

* Outras

Qt5 pode ser instalado diretamente do [site Qt](https://www.qt.io/download), de instaladores não oficiais como [aqtinstall](https://github.com/miurahr/aqtinstall), de repositórios mantidos pela comunidade ou [construídos a partir da fonte](https://wiki.qt.io/Building_Qt_5_from_Git).

Aqui está como alguém pode usar o aqtinstall para instalar o Qt 5.15.2 em uma distribuição baseada no Debian.

```bash
sudo apt-get update
sudo apt-get install libgl1-mesa-dev libxcb-xinerama0 libpulse-dev git python3 python3-pip python3-venv -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.2.*
~/venv/bin/aqt install-qt linux desktop 5.15.2 --outputdir ~/Qt
PATH=~/Qt/5.15.2/gcc_64/bin:$PATH
export PATH
```

#### Processo de compilação

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
cd wiredpanda
cmake -B build
cmake --build build --config Release
```

Este processo pode demorar um pouco. Uma vez concluído, o binário estará localizado em `wiredpanda/build/wiredpanda`, no Linux, e em `wiredpanda/build/wiredpanda.app/Contents/MacOS/wiredpanda` no macOS.

## Licença

wiRedPanda está licenciado sob a [GNU General Public License, Versão 3.0](http://www.gnu.org/licenses/).

Consulte [`LICENSE`](LICENSE) para obter o texto completo da licença.
  
    Copyright (C) 2025 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres e Vinícius Miguel.
    
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
