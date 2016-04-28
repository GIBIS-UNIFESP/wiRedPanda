#ifndef SERIALIZATIONFUNCTIONS_H
#define SERIALIZATIONFUNCTIONS_H

#include <QGraphicsItem>

class Editor;
class Scene;

class SerializationFunctions {
public:
  static void serialize( const QList< QGraphicsItem* > &items, QDataStream &ds );
  static QList< QGraphicsItem* > deserialize( Editor *editor,
                                              QDataStream &ds,
                                              double version,
                                              QString parentFile,
                                              QMap< quint64, QNEPort* > portMap = QMap< quint64, QNEPort* >( ) );
  static QList< QGraphicsItem* > load( Editor *editor, QDataStream &ds, QString parentFile, Scene *scene = nullptr );
};

#endif /* SERIALIZATIONFUNCTIONS_H */
