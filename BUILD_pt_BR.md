# Compilação

## No Windows

Instale o Qt 6.12.0 com o [instalador online do Qt](https://www.qt.io/download), selecionando em Qt 6.12.0 a
compilação MSVC 2022 64-bit e, em suas Additional Libraries, Qt Multimedia e Qt Image Formats.

Em seguida, abra o CMakeLists.txt no Qt Creator e pressione Ctrl+R para compilar e executar.

## No Linux e macOS

### Dependências

O Qt 6.8.0+ com os módulos Multimedia e Svg é necessário para compilar. O Qt recomendado é o 6.12.0, a versão que a CI
testa e que as versões publicadas incluem, instalado com o [aqtinstall](https://github.com/miurahr/aqtinstall), que dá
o mesmo Qt qualquer que seja a versão do seu sistema operacional.

* Debian e Ubuntu

```bash
sudo apt-get update
sudo apt-get install build-essential zlib1g-dev libgl1-mesa-dev libpulse-dev git python3 python3-pip python3-venv \
    libxcb-cursor0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libxkbcommon-x11-0 \
    libxrandr2 libwayland-cursor0 libwayland-egl1 -y
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt linux desktop 6.12.0 -m qtmultimedia qtimageformats --outputdir ~/Qt
PATH=~/Qt/6.12.0/gcc_64/bin:$PATH
export PATH
```

`zlib1g-dev` são os cabeçalhos do zlib, com os quais o wiRedPanda lê seus arquivos. Os pacotes `libxcb-*`,
`libxkbcommon-x11-0`, `libxrandr2` e `libwayland-*` são os que os plugins de plataforma X11 e Wayland do Qt e seu
backend multimídia carregam em tempo de execução; uma instalação de desktop normalmente já os tem. Em outras
distribuições, instale os equivalentes com o seu gerenciador de pacotes e depois execute as cinco últimas linhas.

* macOS (com as ferramentas de linha de comando do Xcode: `xcode-select --install`)

```bash
python3 -m venv ~/venv
~/venv/bin/pip install aqtinstall==3.*
~/venv/bin/aqt install-qt mac desktop 6.12.0 -m qtmultimedia qtimageformats --outputdir ~/Qt
PATH=~/Qt/6.12.0/macos/bin:$PATH
export PATH
```

* A partir dos pacotes da sua distribuição

Se a sua distribuição traz o Qt 6.8 ou posterior (Debian 13, Ubuntu 26.04, Fedora, Arch Linux), os pacotes dela
também funcionam. Debian 12 e Ubuntu 22.04 e 24.04 trazem um Qt mais antigo.

```bash
# Debian 13, Ubuntu 26.04
sudo apt install build-essential zlib1g-dev qt6-base-dev qt6-multimedia-dev qt6-svg-dev
# Baseado em Arch Linux
sudo pacman -S base-devel qt6-base qt6-multimedia qt6-svg
# macOS, com Homebrew
brew install qt6
```

O Qt também pode ser instalado a partir do [site do Qt](https://www.qt.io/download) ou
[construído a partir da fonte](https://wiki.qt.io/Building_Qt_6_from_Git).

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
