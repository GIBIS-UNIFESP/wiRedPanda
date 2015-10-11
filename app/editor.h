#ifndef EDITOR_H
#define EDITOR_H


#include <QObject>
#include <QUndoCommand>
#include <scene.h>

#include "simulationcontroller.h"
#include <elementeditor.h>
#include <elementfactory.h>
#include <nodes/qneconnection.h>

class Editor : public QObject {
  Q_OBJECT
public:
  explicit Editor( QObject *parent = 0 );
  virtual ~Editor( );
  void install( Scene *s );
  void save( QDataStream &ds );
  void load( QDataStream &ds );
  void cut( QDataStream &ds );
  void copy( QDataStream &ds );
  void paste( QDataStream &ds );
  void selectAll( );
signals:
  void scroll( int x, int y );

public slots:
  void clear( );
  void showWires( bool checked );
  void rotate( bool rotateRight );
private:
  QUndoStack *undoStack;
  Scene *scene;
  QGraphicsItem* itemAt( const QPointF &pos );
  QNEConnection *conn;
  ElementFactory factory;
  ElementEditor *elementEditor;
  bool markingSelectionBox;
  QGraphicsRectItem *selectionRect;
  QPointF selectionStartPoint;
  SimulationController *simulationController;
  QPointF mousePos;
  QPointF lastPos;
  void addItem( QGraphicsItem *item );
  QNEPort *m_hoverPort;

  /* QObject interface */
public:
  bool eventFilter( QObject *obj, QEvent *evt );
  void deleteElements( );
  void setElementEditor( ElementEditor *value );
  QUndoStack* getUndoStack( ) const;
  ElementFactory &getFactory( );
  Scene* getScene( ) const;
  void buildSelectionRect( );
};

#endif /* EDITOR_H */
