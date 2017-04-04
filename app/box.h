#ifndef BOX_H
#define BOX_H

#include "elementfactory.h"
#include "graphicelement.h"
#include "scene.h"
#include "simulationcontroller.h"

#include <QFileInfo>
#include <QFileSystemWatcher>

class Editor;

class Box : public GraphicElement {
  Q_OBJECT

  friend class CodeGenerator;
public:
  Box( Editor *editor, QGraphicsItem *parent = 0 );
  virtual ~Box( );
  /* GraphicElement interface */
  virtual ElementType elementType( ) {
    return( ElementType::BOX );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::BOX );
  }
  void save( QDataStream &ds );
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );
  void updateLogic( );
  void loadFile( QString fname );
  QString getFile( ) const;

  QString getParentFile( ) const;
  void setParentFile( const QString &value );


  QFileInfo findFile( QString fname );

  Box* getParentBox( ) const;
  void setParentBox( Box *value );

  void verifyRecursion( QString fname );
  QVector< GraphicElement* > getElements( ) const;

private:
  Editor *editor;
  QString m_file;
  QVector< QNEOutputPort* > inputMap;
  QVector< QNEInputPort* > outputMap;
  QFileSystemWatcher watcher;
  bool isAskingToReload;
  QString parentFile;
  Box *parentBox;
  QVector< GraphicElement* > elements;
public slots:
  void fileChanged( QString file );

  /* QGraphicsItem interface */
protected:
  void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
};

#endif /* BOX_H */
