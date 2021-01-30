find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Core
    Test
)

add_executable(test
    EXCLUDE_FROM_ALL
    main.cpp
    testcommands.cpp
    testelements.cpp
    testfiles.cpp
    testicons.cpp
    testlogicelements.cpp
    testsimulationcontroller.cpp
    testwaveform.cpp
)

target_link_libraries(
    test
PRIVATE
    Qt5::Core
    Qt5::Test
)