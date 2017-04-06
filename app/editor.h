#ifndef EDITOR_H
#define EDITOR_H


#include "simulationcontroller.h"
#include <QObject>
#include <QTime>
#include <QUndoCommand>
#include <elementeditor.h>
#include <elementfactory.h>
#include <memory>
#include <nodes/qneconnection.h>
#include <scene.h>

class Box;
class MainWindow;

class Editor : public QObject {
  Q_OBJECT
  QTime timer;
public:
  explicit Editor( QObject *parent = 0 );
  virtual ~Editor( );
  void save( QDataStream &ds );
  void load( QDataStream &ds );
  void cut( const QList< QGraphicsItem* > &items, QDataStream &ds );
  void copy( const QList< QGraphicsItem* > &items, QDataStream &ds );
  void paste( QDataStream &ds );
  void selectAll( );
signals:
  void scroll( int x, int y );
  void circuitHasChanged( );

public slots:
  void clear( );
  void showWires( bool checked );
  void showGates( bool checked );
  void rotate( bool rotateRight );
  void selectionChanged( );
  void receiveCommand( QUndoCommand *cmd );
  void copyAction( );
  void cutAction( );
  void pasteAction( );
  void deleteAction( );

  void updateTheme( );
private:
  QUndoStack *undoStack;
  Scene *scene;
  QList< QGraphicsItem* > itemsAt( QPointF pos );
  QGraphicsItem* itemAt( QPointF pos );
  int _editedConn_id;
  int _hoverPortElm_id;
  int _hoverPort_nbr;
  ElementEditor *_elementEditor;

  bool markingSelectionBox;
  QGraphicsRectItem *selectionRect;
  QPointF selectionStartPoint;
  SimulationController *simulationController;
  QPointF mousePos, lastPos;
  void addItem( QGraphicsItem *item );
  bool draggingElement;
  QList< GraphicElement* > movedElements;
  QList< QPointF > oldPositions;
  MainWindow *mainWindow;
/*  bool mControlKeyPressed; */
  bool mShowWires;
  bool mShowGates;

  bool mousePressEvt( QGraphicsSceneMouseEvent *mouseEvt );
  bool mouseMoveEvt( QGraphicsSceneMouseEvent *mouseEvt );
  bool mouseReleaseEvt( QGraphicsSceneMouseEvent *mouseEvt );
  bool dropEvt( QGraphicsSceneDragDropEvent *dde );
  bool dragMoveEvt( QGraphicsSceneDragDropEvent *dde );
  bool wheelEvt( QWheelEvent *wEvt );

  void ctrlDrag( QPointF pos );
  void install( Scene *s );

  QNEConnection* getEditedConn( );
  void setEditedConn( QNEConnection *editedConn );

  /* QObject interface */
  void detachConnection( QNEInputPort *endPort );
  void startNewConnection( QNEOutputPort *startPort );
  void startNewConnection( QNEInputPort *endPort );

  void startSelectionRect( );

  void makeConnection( QNEConnection *editedConn );
public:
  bool eventFilter( QObject *obj, QEvent *evt );
  void setElementEditor( ElementEditor *value );
  QUndoStack* getUndoStack( ) const;
  Scene* getScene( ) const;
  void buildSelectionRect( );
  bool loadBox( Box *box, QString fname );

  void handleHoverPort( );
  void releaseHoverPort( );

  void setHoverPort( QNEPort *port );
  QNEPort* getHoverPort( );

  void resizeScene( );
  SimulationController* getSimulationController( ) const;
  void contextMenu( QPoint screenPos );
  void updateVisibility( );
  QPointF getMousePos( ) const;


  ElementEditor* getElementEditor( ) const;

  void deleteEditedConn( );
};

#endif /* EDITOR_H */
