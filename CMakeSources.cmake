set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/app/application.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/arduino/codegenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/bewaveddolphin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/clockdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/commands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/elementeditor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/elementfactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/elementlabel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/elementmapping.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/enums.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/graphicelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/graphicsview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/ic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/itemwithid.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/lengthdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/mainwindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/nodes/qneconnection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/nodes/qneport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/recentfiles.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/scene.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/serialization.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/settings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/simulation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/simulationblocker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/thememanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/trashbutton.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/workspace.cpp

    ${CMAKE_CURRENT_LIST_DIR}/app/element/and.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/audiobox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/buzzer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/clock.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/demux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/dflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/display_14.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/display_16.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/display_7.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/dlatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputbutton.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputgnd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputrotary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputswitch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputvcc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/jkflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/led.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/line.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/mux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/nand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/node.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/nor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/not.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/or.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/srflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/srlatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/text.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/tflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/truth_table.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/xnor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/element/xor.cpp

    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicdemux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicdflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicdlatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicinput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicjkflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicmux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicoutput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicsrflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicsrlatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logictflipflop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logictruthtable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicxnor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicxor.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/app/application.h
    ${CMAKE_CURRENT_LIST_DIR}/app/arduino/codegenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/app/bewaveddolphin.h
    ${CMAKE_CURRENT_LIST_DIR}/app/clockdialog.h
    ${CMAKE_CURRENT_LIST_DIR}/app/commands.h
    ${CMAKE_CURRENT_LIST_DIR}/app/common.h
    ${CMAKE_CURRENT_LIST_DIR}/app/elementeditor.h
    ${CMAKE_CURRENT_LIST_DIR}/app/elementfactory.h
    ${CMAKE_CURRENT_LIST_DIR}/app/elementlabel.h
    ${CMAKE_CURRENT_LIST_DIR}/app/elementmapping.h
    ${CMAKE_CURRENT_LIST_DIR}/app/enums.h
    ${CMAKE_CURRENT_LIST_DIR}/app/globalproperties.h
    ${CMAKE_CURRENT_LIST_DIR}/app/graphicelement.h
    ${CMAKE_CURRENT_LIST_DIR}/app/graphicelementinput.h
    ${CMAKE_CURRENT_LIST_DIR}/app/graphicsview.h
    ${CMAKE_CURRENT_LIST_DIR}/app/ic.h
    ${CMAKE_CURRENT_LIST_DIR}/app/itemwithid.h
    ${CMAKE_CURRENT_LIST_DIR}/app/lengthdialog.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement.h
    ${CMAKE_CURRENT_LIST_DIR}/app/mainwindow.h
    ${CMAKE_CURRENT_LIST_DIR}/app/nodes/qneconnection.h
    ${CMAKE_CURRENT_LIST_DIR}/app/nodes/qneport.h
    ${CMAKE_CURRENT_LIST_DIR}/app/recentfiles.h
    ${CMAKE_CURRENT_LIST_DIR}/app/registertypes.h
    ${CMAKE_CURRENT_LIST_DIR}/app/scene.h
    ${CMAKE_CURRENT_LIST_DIR}/app/serialization.h
    ${CMAKE_CURRENT_LIST_DIR}/app/settings.h
    ${CMAKE_CURRENT_LIST_DIR}/app/simulation.h
    ${CMAKE_CURRENT_LIST_DIR}/app/simulationblocker.h
    ${CMAKE_CURRENT_LIST_DIR}/app/thememanager.h
    ${CMAKE_CURRENT_LIST_DIR}/app/trashbutton.h
    ${CMAKE_CURRENT_LIST_DIR}/app/workspace.h

    ${CMAKE_CURRENT_LIST_DIR}/app/element/and.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/audiobox.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/buzzer.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/clock.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/demux.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/dflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/display_14.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/display_16.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/display_7.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/dlatch.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputbutton.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputgnd.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputrotary.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputswitch.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/inputvcc.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/jkflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/led.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/line.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/mux.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/nand.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/node.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/nor.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/not.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/or.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/srflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/srlatch.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/text.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/tflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/truth_table.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/xnor.h
    ${CMAKE_CURRENT_LIST_DIR}/app/element/xor.h

    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicand.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicdemux.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicdflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicdlatch.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicinput.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicjkflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicmux.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnand.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnode.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnone.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnor.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicnot.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicor.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicoutput.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicsrflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicsrlatch.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logictflipflop.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logictruthtable.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicxnor.h
    ${CMAKE_CURRENT_LIST_DIR}/app/logicelement/logicxor.h
)

set(FORMS
    ${CMAKE_CURRENT_LIST_DIR}/app/bewaveddolphin.ui
    ${CMAKE_CURRENT_LIST_DIR}/app/clockdialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/app/elementeditor.ui
    ${CMAKE_CURRENT_LIST_DIR}/app/lengthdialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/app/mainwindow.ui
)

set(RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/basic/basic.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/dolphin/dolphin.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/input/input.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/memory/dark/memory_dark.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/memory/light/memory_light.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/misc/misc.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/output/output.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/toolbar/toolbar.qrc
    ${CMAKE_CURRENT_LIST_DIR}/app/resources/translations/translations.qrc
)
