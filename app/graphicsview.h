#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

class GraphicsView : public QGraphicsView {
  Q_OBJECT
public:
  explicit GraphicsView( QWidget *parent = 0 );


  /* QWidget interface */
protected:
  void mousePressEvent( QMouseEvent *e );
  void mouseReleaseEvent( QMouseEvent *e );
  void mouseMoveEvent( QMouseEvent *e );
  void keyPressEvent(QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);

private:
  bool _pan;
  bool _space;
  int _panStartX;
  int _panStartY;

};

#endif /* GRAPHICSVIEW_H */
