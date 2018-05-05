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
#include "element/dflipflop.h"
#include "element/display.h"
#include "element/dlatch.h"
#include "element/jkflipflop.h"
#include "element/jklatch.h"
#include "element/srflipflop.h"
#include "element/tflipflop.h"
#include "element/tlatch.h"
#include "buzzer.h"
#include "demux.h"
#include "mux.h"
#include "node.h"
#include "qneconnection.h"

#include <QDebug>

ElementFactory*ElementFactory::instance = new ElementFactory( );


int ElementFactory::getLastId( ) const {
  return( _lastId );
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
         text == "BUZZER" ? ElementType::BUZZER :
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
      case ElementType::BUZZER: return( "BUZZER" );
      case ElementType::UNKNOWN: return ( "UNKNOWN" );
   }

  return( "UNKNOWN" );
}

QString ElementFactory::translatedName( ElementType type ) {
  switch( type ) {
      case ElementType::BUTTON: return( tr( "Button" ) );
      case ElementType::LED: return( tr( "Led" ) );
      case ElementType::AND: return( tr( "And" ) );
      case ElementType::OR: return( tr( "Or" ) );
      case ElementType::CLOCK: return( tr( "Clock" ) );
      case ElementType::SWITCH: return( tr( "Switch" ) );
      case ElementType::NOT: return( tr( "Not" ) );
      case ElementType::NAND: return( tr( "Nand" ) );
      case ElementType::NOR: return( tr( "Nor" ) );
      case ElementType::XOR: return( tr( "Xor" ) );
      case ElementType::XNOR: return( tr( "Xnor" ) );
      case ElementType::VCC: return( tr( "VCC" ) );
      case ElementType::GND: return( tr( "GND" ) );
      case ElementType::DFLIPFLOP: return( tr( "D-flipflop" ) );
      case ElementType::DLATCH: return( tr( "D-latch" ) );
      case ElementType::JKFLIPFLOP: return( tr( "JK-flipflop" ) );
      case ElementType::JKLATCH: return( tr( "JK-latch" ) );
      case ElementType::SRFLIPFLOP: return( tr( "SR-flipflop" ) );
      case ElementType::TLATCH: return( tr( "T-latch" ) );
      case ElementType::TFLIPFLOP: return( tr( "T-flipflop" ) );
      case ElementType::DISPLAY: return( tr( "Display" ) );
      case ElementType::BOX: return( tr( "Box" ) );
      case ElementType::MUX: return( tr( "Mux" ) );
      case ElementType::DEMUX: return( tr( "Demux" ) );
      case ElementType::NODE: return( tr( "Node" ) );
      case ElementType::BUZZER: return( tr( "Buzzer" ) );
      case ElementType::UNKNOWN: return( tr( "Unknown" ) );
  }

  return( tr( "Unknown" ) );
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
      case ElementType::DFLIPFLOP: return( QPixmap( ":/memory/light/D-flipflop.png" ) );
      case ElementType::DLATCH: return( QPixmap( ":/memory/light/D-latch.png" ) );
      case ElementType::JKFLIPFLOP: return( QPixmap( ":/memory/light/JK-flipflop.png" ) );
      case ElementType::JKLATCH: return( QPixmap( ":/memory/light/JK-latch.png" ) );
      case ElementType::SRFLIPFLOP: return( QPixmap( ":/memory/light/SR-flipflop.png" ) );
      case ElementType::TLATCH: return( QPixmap( ":/memory/light/T-latch.png" ) );
      case ElementType::TFLIPFLOP: return( QPixmap( ":/memory/light/T-flipflop.png" ) );
      case ElementType::DISPLAY: return( QPixmap( ":/output/counter/counter_on.png" ) );
      case ElementType::BOX: return( QPixmap( ":/basic/box.png" ) );
      case ElementType::MUX: return( QPixmap( ":/basic/mux.png" ) );
      case ElementType::DEMUX: return( QPixmap( ":/basic/demux.png" ) );
      case ElementType::NODE: return( QPixmap( ":/basic/node.png" ) );
      case ElementType::BUZZER: return( QPixmap( ":/output/BuzzerOff.png" ) );
      case ElementType::UNKNOWN: return( QPixmap( ) );
  }

  return( QPixmap( ) );
}

ElementFactory::ElementFactory( ) {
  clear( );
}

GraphicElement* ElementFactory::buildElement( ElementType type, QGraphicsItem *parent ) {
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
        type == ElementType::BOX ? new Box( parent ) :
        type == ElementType::NODE ? new Node( parent ) :
        type == ElementType::MUX ? new Mux( parent ) :
        type == ElementType::DEMUX ? new Demux( parent ) :
        type == ElementType::BUZZER ? new Buzzer( parent ) :
                                      static_cast<GraphicElement*>(nullptr);
  return( elm );
}

QNEConnection* ElementFactory::buildConnection( QGraphicsItem *parent ) {
  return( new QNEConnection( parent ) );
}

ItemWithId* ElementFactory::getItemById(int id ) {
  if( instance->map.contains( id ) ) {
    return( instance->map[ id ] );
  }
  return( nullptr );
}

bool ElementFactory::contains(int id ) {
  return( instance->map.contains( id ) );
}

void ElementFactory::addItem( ItemWithId *item ) {
  if( item ) {
    int newId = instance->next_id( );
    instance->map[ newId ] = item;
    item->setId( newId );
  }
}

void ElementFactory::removeItem( ItemWithId *item ) {
  instance->map.remove( item->id( ) );
}

void ElementFactory::updateItemId(ItemWithId *item, int newId ) {
  instance->map.remove( item->id( ) );
  instance->map[ newId ] = item;
  item->setId( newId );
}


int ElementFactory::next_id( ) {
  return( _lastId++ );
}

void ElementFactory::clear( ) {
  map.clear( );
  _lastId = 1;
}
