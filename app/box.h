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
  ~Box( );
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

  QVector< GraphicElement* > getElements( ) const;

private:
  Editor *editor;
  QString m_file;

  /* QGraphicsItem interface */
protected:
  void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
};

#endif /* BOX_H */
