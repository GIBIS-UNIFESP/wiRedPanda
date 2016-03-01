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

ElementFactory::ElementFactory( ) {
  lastId = 0;
}

void ElementFactory::giveBackId( size_t id ) {
  available_id.push_back( id );
}

GraphicElement* ElementFactory::buildElement( ElementType type, Editor *editor, QGraphicsItem *parent ) {
  GraphicElement *elm;
  switch( type ) {
  case ElementType::BUTTON:
    elm = new InputButton( parent );
    break;
  case ElementType::SWITCH:
    elm = new InputSwitch( parent );
    break;
  case ElementType::LED:
    elm = new Led( parent );
    break;
  case ElementType::NOT:
    elm = new Not( parent );
    break;
  case ElementType::AND:
    elm = new And( parent );
    break;
  case ElementType::OR:
    elm = new Or( parent );
    break;
  case ElementType::NAND:
    elm = new Nand( parent );
    break;
  case ElementType::NOR:
    elm = new Nor( parent );
    break;
  case ElementType::CLOCK:
    elm = new Clock( parent );
    break;
  case ElementType::XOR:
    elm = new Xor( parent );
    break;
  case ElementType::XNOR:
    elm = new Xnor( parent );
    break;
  case ElementType::VCC:
    elm = new InputVcc( parent );
    break;
  case ElementType::GND:
    elm = new InputGnd( parent );
    break;
  case ElementType::DLATCH:
    elm = new DLatch( parent );
    break;
  case ElementType::DFLIPFLOP:
    elm = new DFlipFlop( parent );
    break;
  case ElementType::JKLATCH:
    elm = new JKLatch( parent );
    break;
  case ElementType::JKFLIPFLOP:
    elm = new JKFlipFlop( parent );
    break;
  case ElementType::SRFLIPFLOP:
    elm = new SRFlipFlop( parent );
    break;
  case ElementType::TFLIPFLOP:
    elm = new TFlipFlop( parent );
    break;
  case ElementType::TLATCH:
    elm = new TLatch( parent );
    break;
  case ElementType::DISPLAY:
    elm = new Display( parent );
    break;
  case ElementType::BOX:
    elm = new Box( editor, parent );
    break;
  case ElementType::NODE:
    elm = new Node( parent );
    break;
  case ElementType::MUX:
    elm = new Mux( parent );
    break;
  case ElementType::DEMUX:
    elm = new Demux( parent );
    break;
  default:
    return( 0 );
    break;
  }
  elm->setId( next_id( ) );
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
