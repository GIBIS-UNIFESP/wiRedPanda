#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H
#include "graphicelement.h"
#include <deque>
class Editor;
class ElementFactory {
  std::deque< size_t > available_id;
  size_t lastId;

public:
  ElementFactory();
  void giveBackId( size_t id );
  GraphicElement * buildElement(ElementType type , Editor * editor, QGraphicsItem * parent = 0);
  size_t getLastId() const;
  static ElementType textToType( QString text );
  static QString typeToText( ElementType type );
  static QPixmap getPixmap( ElementType type );

private:
  size_t next_id();

};

#endif // ELEMENTFACTORY_H
