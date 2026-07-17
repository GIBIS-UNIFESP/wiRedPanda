# wiredpanda_quick: the new Qt Quick-based UI. See the "QUICK SHELL" section of the root
# CMakeLists.txt for how this fits into the qtquick-rewrite migration. Warning flags, C++
# standard, AUTOMOC, etc. are inherited from the root CMakeLists.txt's directory scope;
# nothing needs to be re-set here.

if(EMSCRIPTEN)
    qt_add_executable(wiredpanda_quick Main.cpp)
else()
    add_executable(wiredpanda_quick Main.cpp)
endif()

qt_add_qml_module(wiredpanda_quick
    URI QuickShell
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(wiredpanda_quick PRIVATE ${QUICK_LIBS})
