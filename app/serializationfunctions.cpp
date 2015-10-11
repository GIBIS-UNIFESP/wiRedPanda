#include "serializationfunctions.h"
#include "graphicelement.h"
#include "qneconnection.h"

void SerializationFunctions::serialize( const QList<QGraphicsItem *> &items, QDataStream &ds ) {
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == GraphicElement::Type ) {
      ds << item->type( );
      ds << ( quint64 ) ( ( GraphicElement* ) item )->elementType( );
      ( ( GraphicElement* ) item )->save( ds );
    }
  }
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == QNEConnection::Type ) {
      ds << item->type( );
      ( ( QNEConnection* ) item )->save( ds );
    }
  }
}
