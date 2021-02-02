
set (
    WPANDA_ELEMENT
    and.cpp
    clock.cpp
    demux.cpp
    dflipflop.cpp
    display.cpp
    display_14.cpp
    dlatch.cpp
    inputbutton.cpp
    inputgnd.cpp
    inputswitch.cpp
    inputvcc.cpp
    jkflipflop.cpp
    jklatch.cpp
    led.cpp
    mux.cpp
    nand.cpp
    node.cpp
    nor.cpp
    not.cpp
    or.cpp
    srflipflop.cpp
    tflipflop.cpp
    xnor.cpp
    xor.cpp
    buzzer.cpp
)
    # tlatch.cpp
add_library(wpandaelements ${WPANDA_ELEMENT} )
target_link_libraries(wpandaelements PUBLIC ${WPANDA_LIBS} )
