# Compilação

## No Windows

Selecione estes componentes para Qt6:
![qt6](https://github.com/user-attachments/assets/aedc8749-8b5c-4f4d-9c65-651b703dccea)

Em seguida, abra o CMakeLists.txt no Qt Creator e pressione Ctrl+R para compilar e executar.

## No Linux e macOS

### Dependências

Qt 6.2.0+ e o módulo QtMultimedia são necessários para compilar.

Em distribuições como Arch Linux, Gentoo, Manjaro, Debian, etc., o Qt 6.2+ pode ser instalado a partir dos repositórios padrão.

* Debian/Ubuntu

```bash
sudo apt install build-essential qt6-base-dev qt6-multimedia-dev libqt6svg6-dev
```

* Baseado em Arch Linux

```bash
sudo pacman -S base-devel qt6-base qt6-multimedia qt6-svg
```

* macOS

```bash
brew install qt6
```

* Outras

Qt6 pode ser instalado diretamente do [site Qt](https://www.qt.io/download), de
instaladores não oficiais como [aqtinstall](https://github.com/miurahr/aqtinstall),
de repositórios mantidos pela comunidade ou
[construídos a partir da fonte](https://wiki.qt.io/Building_Qt_6_from_Git).

Aqui está como alguém pode usar o aqtinstall para instalar o Qt 6.11.0 em uma
distribuição baseada no Debian.

```bash
sudo apt-get update
sudo apt-get install libgl1-mesa-dev libxcb-xinerama0 libpulse-dev git python3 python3-pip python3-venv -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt linux desktop 6.11.0 --outputdir ~/Qt
PATH=~/Qt/6.11.0/gcc_64/bin:$PATH
export PATH
```

### Processo de compilação

**Requisitos**: Instale o sistema de compilação Ninja:

* Windows: `choco install ninja` ou `scoop install ninja`
* Linux: `apt install ninja-build` ou `yum install ninja-build`
* macOS: `brew install ninja`

**Comandos de compilação (todas as plataformas):**

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda
cd wiredpanda
cmake --preset release
cmake --build --preset release
```

Este processo pode demorar um pouco. Uma vez concluído, o binário estará localizado em `wiredpanda/build/wiredpanda`, no Linux, e em `wiredpanda/build/wiredpanda.app/Contents/MacOS/wiredpanda` no macOS.

## Licença

wiRedPanda está licenciado sob a [GNU General Public License, Versão 3.0](http://www.gnu.org/licenses/).

Consulte [`LICENSE`](LICENSE) para obter o texto completo da licença.

```text
Copyright (C) 2026 - Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres e Vinícius Miguel.

Este programa é um software livre: você pode redistribuí-lo e/ou modificá-lo
sob os termos da Licença Pública Geral GNU conforme publicada pela
Free Software Foundation, seja a versão 3 da Licença, ou
(a seu critério) qualquer versão posterior.

Este programa é distribuído na esperança de que seja útil,
mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de
COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM. Veja a
Licença Pública Geral GNU para mais detalhes.

Você deve ter recebido uma cópia da Licença Pública Geral GNU
junto com este programa. Caso contrário, consulte http://www.gnu.org/licenses/.
```
