find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Charts
    Core
    Widgets
    Test
)

include_directories(../app)
include_directories(../app/element)
include_directories(../app/nodes)

add_executable(
    wiredtest
    main.cpp
    testcommands.cpp
    testelements.cpp
    testfiles.cpp
    testicons.cpp
    testlogicelements.cpp
    testsimulationcontroller.cpp
    testwaveform.cpp
)

target_compile_definitions(
    wiredtest
    PUBLIC
        -DCURRENTDIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    wiredtest
PRIVATE
    Qt5::Core
    Qt5::Charts
    Qt5::Widgets
    Qt5::Test
)