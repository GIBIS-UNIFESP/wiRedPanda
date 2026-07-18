# wiredpanda_quick: the new Qt Quick-based UI. See the "QUICK SHELL" section of the root
# CMakeLists.txt for how this fits into the qtquick-rewrite migration. Warning flags, C++
# standard, AUTOMOC, etc. are inherited from the root CMakeLists.txt's directory scope;
# nothing needs to be re-set here.

set(QUICK_SHELL_SOURCES
    Canvas/CanvasCommands.cpp
    Canvas/CanvasCommands.h
    Canvas/CanvasICRegistry.cpp
    Canvas/CanvasICRegistry.h
    Canvas/CanvasItem.cpp
    Canvas/CanvasItem.h
    Canvas/SpatialIndex.cpp
    Canvas/SpatialIndex.h
    Canvas/TextureAtlas.cpp
    Canvas/TextureAtlas.h
    Main.cpp
)

if(EMSCRIPTEN)
    qt_add_executable(wiredpanda_quick ${QUICK_SHELL_SOURCES})
else()
    add_executable(wiredpanda_quick ${QUICK_SHELL_SOURCES})
endif()

qt_add_qml_module(wiredpanda_quick
    URI QuickShell
    VERSION 1.0
    QML_FILES Main.qml
)

# CanvasItem constructs real GraphicElement subclasses (And/InputSwitch/Led) and drives them
# through the real Simulation engine (see CanvasItem.h's doc comment) -- it needs the actual
# domain/simulation classes and their embedded SVG icon resources (ElementAppearance::setPixmap()
# loads them at construction time and throws if they're missing), so this target whole-archives
# wiredpanda_lib exactly as wiredpanda/test_wiredpanda already do. This is expected, temporary
# coupling through the migration, not a regression: wiredpanda_lib itself doesn't stop being
# Widgets-coupled until the plan's Phase 8 cutover, and wiredpanda_quick reusing its real,
# unmodified domain classes is precisely the point of Phase 1's "wired to the real Simulation
# engine, not synthetic data" goal.
link_wiredpanda_whole_archive(wiredpanda_quick)
target_link_libraries(wiredpanda_quick PRIVATE ${QUICK_LIBS} wiredpanda_resources)
