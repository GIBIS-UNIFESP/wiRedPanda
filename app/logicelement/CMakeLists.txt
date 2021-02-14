
set (
    WPANDA_LOGICELEMENT
    logicnode.cpp
    logicinput.cpp
    logicoutput.cpp
    logicand.cpp
    logicor.cpp
    logicnand.cpp
    logicnor.cpp
    logicxor.cpp
    logicxnor.cpp
    logicnot.cpp
    logicjkflipflop.cpp
    logicsrflipflop.cpp
    logictflipflop.cpp
    logicdflipflop.cpp
    logicdlatch.cpp
    logicmux.cpp
    logicdemux.cpp
)

add_library(wpandalogicelement ${WPANDA_LOGICELEMENT} )
target_link_libraries(wpandalogicelement PUBLIC ${WPANDA_LIBS} )
target_compile_options(wpandalogicelement PRIVATE ${COMPILE_WARNS} )
