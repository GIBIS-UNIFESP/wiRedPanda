#ifndef BOX_H
#define BOX_H

#include "elementfactory.h"
#include "graphicelement.h"
#include "scene.h"
#include "simulationcontroller.h"

#include <QFileSystemWatcher>
#include <QMutex>

class Box : public GraphicElement {

public:
  Box( ElementFactory *factory, QGraphicsItem *parent = 0 );
  ~Box( );
  /* GraphicElement interface */
  ElementType elementType( );
  void save( QDataStream &ds );
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );
  void updateLogic( );
  void loadFile( QString fname );
  QString getFile() const;

  QString getParentFile( ) const;
  void setParentFile( const QString &value );


  private:
  ElementFactory *factory;
  QString m_file;
  QVector< QNEPort* > inputMap;
  QVector< QNEPort* > outputMap;
  Scene myScene;
  SimulationController simulationController;
  QFileSystemWatcher watcher;
  QMutex mutex;
  bool isAskingToReload;
  QString parentFile;
  void sortMap( QVector< QNEPort* > &map );

public slots:
  void fileChanged( QString file );

  /* QGraphicsItem interface */
protected:
  void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
};

#endif /* BOX_H */
