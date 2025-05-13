_Leia-me em [português](README_pt_BR.md). Read me in [english](README.md)._

# wiRedPanda ![Todos los lanzamientos de GitHub](https://img.shields.io/github/downloads/gibis-unifesp/wiredpanda/total?style=flat-square) ![Lanzamiento de GitHub (último por fecha)](https://img.shields.io/github/v/release/gibis-unifesp/wiredpanda?style=flat-square) ![GitHub](https://img.shields.io/github/license/gibis-unifesp/wiredpanda?style=flat-square) [![codecov](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda/branch/master/graph/badge.svg?token=5YBYB4J705)](https://codecov.io/gh/GIBIS-UNIFESP/wiRedPanda) <a href="https://hosted.weblate.org/engage/wiredpanda/"><img src="https://hosted.weblate.org/widget/wiredpanda/svg-badge.svg" alt="Estado de la traducción" /></a>


wiRedPanda es un software gratuito diseñado para ayudar a los estudiantes a aprender sobre circuitos lógicos y simularlos de una manera fácil y amigable.

Las principales características del software son:
  - Funciona en Windows, macOS y Linux;
  - Simulación lógica en tiempo real;
  - Interfaz amigable;
  - Es intuitivo y fácil de usar;
  - Exporta tu trabajo como una imagen o un PDF.

![Pantalla principal](https://gibis-unifesp.github.io/wiRedPanda/demo.gif)

## Descargas
Los binarios compilados para Windows, Linux y macOS están disponibles [aquí](https://github.com/GIBIS-UNIFESP/wiRedPanda/releases).

## Compilando

```bash
git clone https://github.com/GIBIS-UNIFESP/wiredpanda.git
mkdir wiredpanda/build
cd wiredpanda/build
qmake ../WPanda.pro
make -j
```

Este proceso podría llevar un tiempo. Una vez concluido, el binario se ubicará en `wiredpanda/build/app/wiredpanda`, en Linux, y en `wiredpanda/build/app/wiredpanda.app/Contents/MacOS/wiredpanda` en macOS.

Para obtener instrucciones de compilación detalladas, consulte [BUILD.md](BUILD.md) o visite la [wiki](https://github.com/GIBIS-UNIFESP/wiRedPanda/wiki/How-to-setup-environment).

## Traducción

Puedes ayudar a traducir wiRedPanda a otros idiomas. Utilizamos la plataforma [Weblate](https://hosted.weblate.org/projects/wiredpanda/wiredpanda) para gestionar las traducciones.

También puedes contribuir bifurcando este repositorio y añadiendo las traducciones manualmente.

Las traducciones se encuentran en el directorio [app/resources/translations](https://github.com/GIBIS-UNIFESP/wiRedPanda/tree/master/app/resources/translations).

<a href="https://hosted.weblate.org/engage/wiredpanda/">
<img src="https://hosted.weblate.org/widget/wiredpanda/wiredpanda/multi-auto.svg" alt="Estado de la traducción" />
</a>

## Estrellas

<a href="https://www.star-history.com/#GIBIS-UNIFESP/wiRedPanda&Date"> 
<imagen> 
<source media="(prefiere-color-scheme: oscuro)" srcset="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date&theme=dark" /> 
<fuente media="(prefiere-color-esquema: claro)" srcset="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date" /> 
<img alt="Gráfico histórico de estrellas" src="https://api.star-history.com/svg?repos=GIBIS-UNIFESP/wiRedPanda&type=Date" /> 
</imagen>
</a>

[![Estado del empaquetado](https://repology.org/badge/vertical-allrepos/wiredpanda.svg)](https://repology.org/project/wiredpanda/versions)

## Licencia

wiRedPanda tiene licencia bajo la [Licencia pública general de GNU, versión 3.0] (http://www.gnu.org/licenses/).

Consulte [`LICENCIA`](LICENCIA) para obtener el texto completo de la licencia.
  
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
    junto con este programa. Si no, consulte <http://www.gnu.org/licenses/>.
