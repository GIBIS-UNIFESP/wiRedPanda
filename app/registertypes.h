#include "and.h"
#include "audiobox.h"
#include "buzzer.h"
#include "clock.h"
#include "demux.h"
#include "dflipflop.h"
#include "display_14.h"
#include "display_16.h"
#include "display_7.h"
#include "dlatch.h"
#include "ic.h"
#include "inputbutton.h"
#include "inputgnd.h"
#include "inputrotary.h"
#include "inputswitch.h"
#include "inputvcc.h"
#include "jkflipflop.h"
#include "led.h"
#include "line.h"
#include "mux.h"
#include "nand.h"
#include "node.h"
#include "nor.h"
#include "not.h"
#include "or.h"
#include "qneconnection.h"
#include "qneport.h"
#include "srflipflop.h"
#include "srlatch.h"
#include "text.h"
#include "tflipflop.h"
#include "truth_table.h"
#include "xnor.h"
#include "xor.h"

inline void registerTypes() {
    qRegisterMetaType<And>();
    qRegisterMetaType<AudioBox>();
    qRegisterMetaType<Buzzer>();
    qRegisterMetaType<Clock>();
    qRegisterMetaType<DFlipFlop>();
    qRegisterMetaType<DLatch>();
    qRegisterMetaType<Demux>();
    qRegisterMetaType<Display14>();
    qRegisterMetaType<Display16>();
    qRegisterMetaType<Display7>();
    qRegisterMetaType<GraphicElement>();
    qRegisterMetaType<IC>();
    qRegisterMetaType<InputButton>();
    qRegisterMetaType<InputGnd>();
    qRegisterMetaType<InputRotary>();
    qRegisterMetaType<InputSwitch>();
    qRegisterMetaType<InputVcc>();
    qRegisterMetaType<JKFlipFlop>();
    qRegisterMetaType<Led>();
    qRegisterMetaType<Line>();
    qRegisterMetaType<Mux>();
    qRegisterMetaType<Nand>();
    qRegisterMetaType<Node>();
    qRegisterMetaType<Nor>();
    qRegisterMetaType<Not>();
    qRegisterMetaType<Or>();
    qRegisterMetaType<QNEConnection>();
    qRegisterMetaType<QNEInputPort>();
    qRegisterMetaType<QNEOutputPort>();
    qRegisterMetaType<SRFlipFlop>();
    qRegisterMetaType<SRLatch>();
    qRegisterMetaType<TFlipFlop>();
    qRegisterMetaType<Text>();
    qRegisterMetaType<TruthTable>();
    qRegisterMetaType<Xnor>();
    qRegisterMetaType<Xor>();
}
