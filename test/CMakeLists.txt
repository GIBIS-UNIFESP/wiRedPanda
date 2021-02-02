
set (
    WPANDA_TEST_FILES
    main.cpp
    testcommands.cpp
    testelements.cpp
    testfiles.cpp
    testicons.cpp
    testlogicelements.cpp
    testsimulationcontroller.cpp
    testwaveform.cpp
)

qt5_add_resources(
    WPANDA_RESOURCES
    ../app/resources/dolphin/dolphin.qrc
    ../app/resources/resources.qrc
    ../app/resources/basic/basic.qrc
    ../app/resources/input/input.qrc
    ../app/resources/output/output.qrc
    ../app/resources/toolbar/toolbar.qrc
    ../app/resources/memory/dark/memory_dark.qrc
    ../app/resources/memory/light/memory_light.qrc
)

add_executable(
    wiredtest
    ${WPANDA_TEST_FILES}
    ${WPANDA_RESOURCES}
)

target_link_libraries(
    wiredtest
PUBLIC
    ${WPANDA_LIBS}
    wpandacommon
    wpandaelements
    wpandalogicelement
    Qt5::Test
)
