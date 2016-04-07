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

QString ElementFactory::typeToText( ElementType type ) {
  switch( type ) {
      case ElementType::BUTTON: return( "BUTTON" );
      case ElementType::LED: return( "LED" );
      case ElementType::AND: return( "AND" );
      case ElementType::OR: return( "OR" );
      case ElementType::CLOCK: return( "CLOCK" );
      case ElementType::SWITCH: return( "SWITCH" );
      case ElementType::NOT: return( "NOT" );
      case ElementType::NAND: return( "NAND" );
      case ElementType::NOR: return( "NOR" );
      case ElementType::XOR: return( "XOR" );
      case ElementType::XNOR: return( "XNOR" );
      case ElementType::VCC: return( "VCC" );
      case ElementType::GND: return( "GND" );
      case ElementType::DFLIPFLOP: return( "DFLIPFLOP" );
      case ElementType::DLATCH: return( "DLATCH" );
      case ElementType::JKFLIPFLOP: return( "JKFLIPFLOP" );
      case ElementType::JKLATCH: return( "JKLATCH" );
      case ElementType::SRFLIPFLOP: return( "SRFLIPFLOP" );
      case ElementType::TLATCH: return( "TLATCH" );
      case ElementType::TFLIPFLOP: return( "TFLIPFLOP" );
      case ElementType::DISPLAY: return( "DISPLAY" );
      case ElementType::BOX: return( "BOX" );
      case ElementType::MUX: return( "MUX" );
      case ElementType::DEMUX: return( "DEMUX" );
      case ElementType::NODE: return( "NODE" );
      case ElementType::UNKNOWN: default: return( "UNKNOWN" );
  }
}

QPixmap ElementFactory::getPixmap( ElementType type ) {
  switch( type ) {
      case ElementType::BUTTON: return( QPixmap( ":/input/buttonOff.png" ) );
      case ElementType::LED: return( QPixmap( ":/output/WhiteLedOff.png" ) );
      case ElementType::AND: return( QPixmap( ":/basic/and.png" ) );
      case ElementType::OR: return( QPixmap( ":/basic/or.png" ) );
      case ElementType::CLOCK: return( QPixmap( ":/input/clock1.png" ) );
      case ElementType::SWITCH: return( QPixmap( ":/input/switchOn.png" ) );
      case ElementType::NOT: return( QPixmap( ":/basic/not.png" ) );
      case ElementType::NAND: return( QPixmap( ":/basic/nand.png" ) );
      case ElementType::NOR: return( QPixmap( ":/basic/nor.png" ) );
      case ElementType::XOR: return( QPixmap( ":/basic/xor.png" ) );
      case ElementType::XNOR: return( QPixmap( ":/basic/xnor.png" ) );
      case ElementType::VCC: return( QPixmap( ":/input/1.png" ) );
      case ElementType::GND: return( QPixmap( ":/input/0.png" ) );
      case ElementType::DFLIPFLOP: return( QPixmap( ":/memory/D-flipflop.png" ) );
      case ElementType::DLATCH: return( QPixmap( ":/memory/D-latch.png" ) );
      case ElementType::JKFLIPFLOP: return( QPixmap( ":/memory/JK-flipflop.png" ) );
      case ElementType::JKLATCH: return( QPixmap( ":/memory/JK-latch.png" ) );
      case ElementType::SRFLIPFLOP: return( QPixmap( ":/memory/SR-flipflop.png" ) );
      case ElementType::TLATCH: return( QPixmap( ":/memory/T-latch.png" ) );
      case ElementType::TFLIPFLOP: return( QPixmap( ":/memory/T-flipflop.png" ) );
      case ElementType::DISPLAY: return( QPixmap( ":/output/counter/counter_on.png" ) );
      case ElementType::BOX: return( QPixmap( ":/basic/box.png" ) );
      case ElementType::MUX: return( QPixmap( ":/basic/mux.png" ) );
      case ElementType::DEMUX: return( QPixmap( ":/basic/demux.png" ) );
      case ElementType::NODE: return( QPixmap( ":/basic/node.png" ) );
      case ElementType::UNKNOWN: return( QPixmap( ) );
  }
  return( QPixmap( ) );
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
