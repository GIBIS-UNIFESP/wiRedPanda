#include "elementfactory.h"
#include "element/and.h"
#include "element/clock.h"
#include "element/inputbutton.h"
#include "element/led.h"
#include "element/or.h"
#include "element/inputswitch.h"
#include "element/not.h"
#include "element/nand.h"
#include "element/nor.h"
#include "element/xor.h"
#include "element/xnor.h"


#include <QDebug>
size_t ElementFactory::getLastId() const {
  return lastId;
}

ElementFactory::ElementFactory() {
  lastId = 0;
}

void ElementFactory::giveBackId(size_t id) {
  available_id.push_back(id);
}

GraphicElement *ElementFactory::buildElement(ElementType type, QGraphicsItem * parent) {
  GraphicElement * elm;
  switch (type) {
//  case ElementType::EMPTY:
//    elm = new Empty();
//    break;
  case ElementType::BUTTON:
    elm = new InputButton(parent);
    break;
  case ElementType::SWITCH:
    elm = new InputSwitch(parent);
    break;
  case ElementType::LED:
    elm = new Led(parent);
    break;
  case ElementType::NOT:
    elm = new Not(parent);
    break;
  case ElementType::AND:
    elm = new And(parent);
    break;
  case ElementType::OR:
    elm = new Or(parent);
    break;
  case ElementType::NAND:
    elm = new Nand(parent);
    break;
  case ElementType::NOR:
    elm = new Nor(parent);
    break;
  case ElementType::CLOCK:
    elm = new Clock(parent);
    break;
  case ElementType::XOR:
    elm = new Xor(parent);
    break;
  case ElementType::XNOR:
    elm = new Xnor(parent);
    break;
//  case ElementType::DLATCH:
//    elm = new DLatch();
//    break;
//  case ElementType::SRLATCH:
//    elm = new SRLatch();
//    break;
//  case ElementType::SCRLATCH:
//    elm = new SCRLatch();
//    break;
  default:
//    std::string msg ( std::string(__FILE__) + ": " + std::to_string(__LINE__)
//                      + ": " + "Elementfactory" + "::" + std::string(__FUNCTION__)
//                      + ": error: Unknown Type!" );
//    throw( std::runtime_error( msg ));
    return 0;
//    elm = new GraphicElement(0,0,0,0,parent);
    break;
  }
  elm->setId( next_id() );
  qDebug() << "Building element " << elm->id();
  return( elm );
}

size_t ElementFactory::next_id() {
  size_t nextId = lastId;
  if (available_id.empty()) {
    lastId ++;
  } else {
    nextId = available_id.front();
    available_id.pop_front();
  }
  return (nextId);
}
