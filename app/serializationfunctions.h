#ifndef SERIALIZATIONFUNCTIONS_H
#define SERIALIZATIONFUNCTIONS_H

#include <QGraphicsItem>

class Editor;

class SerializationFunctions {
public:
  static void serialize( const QList< QGraphicsItem* > &items, QDataStream &ds );
  static QList< QGraphicsItem* > deserialize(Editor * editor, QDataStream &ds );
};

#endif /* SERIALIZATIONFUNCTIONS_H */
