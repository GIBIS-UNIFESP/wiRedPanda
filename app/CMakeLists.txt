
set(
    WPANDA_FILES
    BewavedDolphin.cpp
    clockDialog.cpp
    commands.cpp
    common.cpp
    editor.cpp
    elementeditor.cpp
    elementfactory.cpp
    elementmapping.cpp
    filehelper.cpp
    globalproperties.cpp
    graphicelement.cpp
    graphicsview.cpp
    graphicsviewzoom.cpp
    ic.cpp
    icmanager.cpp
    icmapping.cpp
    icnotfoundexception.cpp
    icprototype.cpp
    icprototypeimpl.cpp
    itemwithid.cpp
    label.cpp
    LengthDialog.cpp
    listitemwidget.cpp
    logicelement.cpp
    mainwindow.cpp
    recentfilescontroller.cpp
    scene.cpp
    scstop.cpp
    serializationfunctions.cpp
    simplewaveform.cpp
    simulationcontroller.cpp
    thememanager.cpp

    arduino/codegenerator.cpp

    nodes/qneconnection.cpp
    nodes/qneport.cpp
)

add_library(wpandacommon ${WPANDA_FILES} )
target_link_libraries( wpandacommon PUBLIC ${WPANDA_LIBS} )
target_compile_definitions(
    wpandacommon
    PUBLIC
        -DCURRENTDIR="${CMAKE_CURRENT_SOURCE_DIR}"
        -DAPP_VERSION="${APP_VERSION}"
)
target_compile_options( wpandacommon PUBLIC ${COMPILE_WARNS} )

add_subdirectory(element)
add_subdirectory(logicelement)

qt5_add_resources(
    WPANDA_RESOURCES
    resources/dolphin/dolphin.qrc
    resources/resources.qrc
    resources/basic/basic.qrc
    resources/input/input.qrc
    resources/output/output.qrc
    resources/toolbar/toolbar.qrc
    resources/memory/dark/memory_dark.qrc
    resources/memory/light/memory_light.qrc
)

set (
    WPANDA_FORMS
    BewavedDolphin.ui
    clockDialog.ui
    LengthDialog.ui
    mainwindow.ui
    elementeditor.ui
    simplewaveform.ui
)

add_executable(
    wpanda
    main.cpp
    ${WPANDA_FORMS}
    ${WPANDA_RESOURCES}
)

target_compile_definitions(
    wpanda
    PUBLIC
    -DAPP_VERSION="${APP_VERSION}"
)
target_compile_options( wpanda PUBLIC ${COMPILE_WARNS} )

target_link_libraries(
    wpanda
PUBLIC
    ${WPANDA_LIBS}
    wpandacommon
    wpandaelements
    wpandalogicelement
)
