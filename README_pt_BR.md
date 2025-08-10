_Read me in [english](README.md). Léame en [español](README_es.md)._

# wiRedPanda ![GitHub All Releases](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square) [![codecov](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda/branch/master/graph/badge.svg?token=5YBYB4J705)](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda) <a href="https://hosted.weblate.org/engage/wiredpanda/"><img src="https://hosted.weblate.org/widget/wiredpanda/svg-badge.svg" alt="Status da tradução" /></a>


wiRedPanda é um software gratuito projetado para ajudar os alunos a aprender sobre circuitos lógicos e simulá-los de maneira fácil e amigável.

As principais características do software são:
  - Funciona em Windows, macOS e Linux;
  - Simulação lógica em tempo real;
  - Interface amigável;
  - É intuitivo e fácil de usar;
  - Exporte seu trabalho como imagem ou PDF.

![Tela principal](https://gibis-unifesp.github.io/wiRedPanda/demo.gif)

## Downloads
Binários compilados para Windows, Linux e macOS estão disponíveis [aqui](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Compilação

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
cd wiredpanda
cmake -B build
cmake --build build --config Release
```

Este processo pode demorar um pouco. Uma vez concluído, o binário estará localizado em `wiredpanda/build/wiredpanda`, no Linux, e em `wiredpanda/build/wiredpanda.app/Contents/MacOS/wiredpanda` no macOS.

Para obter instruções detalhadas de construção, verifique [BUILD.md](BUILD.md) ou visite o [wiki](https://github.com/GIBIS-UNIFESP/wiRedPanda/wiki/How-to-setup-environment).

## Tradução

Você pode ajudar a traduzir o wiRedPanda para outros idiomas. Usamos a plataforma [Weblate](https://hosted.weblate.org/projects/wiredpanda/wiredpanda) para gerenciar as traduções.

Como alternativa, você também pode contribuir bifurcando este repositório e adicionando traduções manualmente.

As traduções estão localizadas no diretório [app/resources/translations](https://github.com/GIBIS-UNIFESP/wiRedPanda/tree/master/app/resources/translations).

<a href="https://hosted.weblate.org/engage/wiredpanda/">
<img src="https://hosted.weblate.org/widget/wiredpanda/wiredpanda/multi-auto.svg" alt="Status da tradução" />
</a>

## Estrelas

<a href="https://www.star-history.com/#GIBIS-UNIFESP/wiRedPanda&Date">
<imagem>
<source media="(prefere esquema de cores: escuro)" srcset="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date&theme=dark" />
<source media="(prefere esquema de cores: light)" srcset="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date" />
<img alt="Gráfico Histórico Estelar" src="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date" />
</imagem>
</a>

[![Status do pacote](https://repology.org/badge/vertical-allrepos/wiredpanda.svg)](https://repology.org/project/wiredpanda/versions)

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
