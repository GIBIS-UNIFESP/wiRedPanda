#include "box.h"
#include "element/and.h"
#include "element/clock.h"
#include "element/inputbutton.h"
#include "element/inputgnd.h"
#include "element/inputswitch.h"
#include "element/inputvcc.h"
#include "element/led.h"
#include "element/nand.h"
#include "element/nor.h"
#include "element/not.h"
#include "element/or.h"
#include "element/xnor.h"
#include "element/xor.h"
#include "elementfactory.h"
#include <element/dflipflop.h>
#include <element/display.h>
#include <element/dlatch.h>
#include <element/jkflipflop.h>
#include <element/jklatch.h>
#include <element/srflipflop.h>
#include <element/tflipflop.h>
#include <element/tlatch.h>


#include <QDebug>
#include <demux.h>
#include <mux.h>
#include <node.h>


size_t ElementFactory::getLastId( ) const {
  return( lastId );
}

ElementType ElementFactory::textToType( QString text ) {
  text = text.toUpper( );
  ElementType type;
  type = text == "BUTTON" ? ElementType::BUTTON :
         text == "LED" ? ElementType::LED :
         text == "AND" ? ElementType::AND :
         text == "OR" ? ElementType::OR :
         text == "CLOCK" ? ElementType::CLOCK :
         text == "SWITCH" ? ElementType::SWITCH :
         text == "NOT" ? ElementType::NOT :
         text == "NAND" ? ElementType::NAND :
         text == "NOR" ? ElementType::NOR :
         text == "XOR" ? ElementType::XOR :
         text == "XNOR" ? ElementType::XNOR :
         text == "VCC" ? ElementType::VCC :
         text == "GND" ? ElementType::GND :
         text == "DFLIPFLOP" ? ElementType::DFLIPFLOP :
         text == "DLATCH" ? ElementType::DLATCH :
         text == "JKFLIPFLOP" ? ElementType::JKFLIPFLOP :
         text == "JKLATCH" ? ElementType::JKLATCH :
         text == "SRFLIPFLOP" ? ElementType::SRFLIPFLOP :
         text == "TLATCH" ? ElementType::TLATCH :
         text == "TFLIPFLOP" ? ElementType::TFLIPFLOP :
         text == "DISPLAY" ? ElementType::DISPLAY :
         text == "BOX" ? ElementType::BOX :
         text == "MUX" ? ElementType::MUX :
         text == "DEMUX" ? ElementType::DEMUX :
         text == "NODE" ? ElementType::NODE :
         ElementType::UNKNOWN;
  return( type );
}

ElementFactory::ElementFactory( ) {
  lastId = 0;
}

void ElementFactory::giveBackId( size_t id ) {
  available_id.push_back( id );
}

GraphicElement* ElementFactory::buildElement( ElementType type, Editor *editor, QGraphicsItem *parent ) {
  GraphicElement *elm;
  elm = type == ElementType::BUTTON ? new InputButton( parent ) :
        type == ElementType::SWITCH ? new InputSwitch( parent ) :
        type == ElementType::LED ? new Led( parent ) :
        type == ElementType::NOT ? new Not( parent ) :
        type == ElementType::AND ? new And( parent ) :
        type == ElementType::OR ? new Or( parent ) :
        type == ElementType::NAND ? new Nand( parent ) :
        type == ElementType::NOR ? new Nor( parent ) :
        type == ElementType::CLOCK ? new Clock( parent ) :
        type == ElementType::XOR ? new Xor( parent ) :
        type == ElementType::XNOR ? new Xnor( parent ) :
        type == ElementType::VCC ? new InputVcc( parent ) :
        type == ElementType::GND ? new InputGnd( parent ) :
        type == ElementType::DLATCH ? new DLatch( parent ) :
        type == ElementType::DFLIPFLOP ? new DFlipFlop( parent ) :
        type == ElementType::JKLATCH ? new JKLatch( parent ) :
        type == ElementType::JKFLIPFLOP ? new JKFlipFlop( parent ) :
        type == ElementType::SRFLIPFLOP ? new SRFlipFlop( parent ) :
        type == ElementType::TFLIPFLOP ? new TFlipFlop( parent ) :
        type == ElementType::TLATCH ? new TLatch( parent ) :
        type == ElementType::DISPLAY ? new Display( parent ) :
        type == ElementType::BOX ? new Box( editor, parent ) :
        type == ElementType::NODE ? new Node( parent ) :
        type == ElementType::MUX ? new Mux( parent ) :
        type == ElementType::DEMUX ? new Demux( parent ) :
        ( GraphicElement* ) nullptr;
  if( elm ) {
    elm->setId( next_id( ) );
  }
  return( elm );
}

size_t ElementFactory::next_id( ) {
  size_t nextId = lastId;
  if( available_id.empty( ) ) {
    lastId++;
  }
  else {
    nextId = available_id.front( );
    available_id.pop_front( );
  }
  return( nextId );
}
