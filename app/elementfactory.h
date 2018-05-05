#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H

#include "graphicelement.h"

#include <deque>
#include <QObject>

class Editor;
class ElementFactory : public QObject {
  Q_OBJECT

  int _lastId;

public:
  static ElementFactory *instance;
  QMap< int, ItemWithId *> map;

  static ElementType textToType( QString text );
  static QString typeToText( ElementType type );
  static QString translatedName(ElementType type);
  static QPixmap getPixmap( ElementType type );
  static GraphicElement* buildElement(ElementType type, QGraphicsItem *parent = nullptr );
  static QNEConnection* buildConnection( QGraphicsItem *parent = nullptr );
  static ItemWithId * getItemById(int id);
  static bool contains( int id );
  static void updateItemId(ItemWithId * item, int newId );
  static void removeItem(ItemWithId * item);
  static void addItem(ItemWithId * item);

  int getLastId( ) const;
  int next_id( );
  void clear();

private:
  ElementFactory( );
};

#endif /* ELEMENTFACTORY_H */
