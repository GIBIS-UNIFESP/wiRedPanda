#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H
#include "graphicelement.h"
#include <deque>

class Editor;
class ElementFactory {
  size_t _lastId;
  static ElementFactory *_instance;

public:
  QMap< size_t, ItemWithId *> map;

  static ElementFactory* instance( );
  static ElementType textToType( QString text );
  static QString typeToText( ElementType type );
  static QPixmap getPixmap( ElementType type );
  static GraphicElement* buildElement( ElementType type, Editor *editor, QGraphicsItem *parent = 0 );
  static QNEConnection* buildConnection( QGraphicsItem *parent = 0 );
  static ItemWithId * getItemById(size_t id);
  static bool contains( size_t id );
  static void updateItemId(ItemWithId * item, size_t newId );
  static void removeItem(ItemWithId * item);
  static void addItem(ItemWithId * item);

  size_t getLastId( ) const;
  size_t next_id( );
  void clear();

  private:
  ElementFactory( );
};

#endif /* ELEMENTFACTORY_H */
