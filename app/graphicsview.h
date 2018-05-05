#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

class GraphicsViewZoom;

class GraphicsView : public QGraphicsView {
  Q_OBJECT
public:
  explicit GraphicsView( QWidget *parent = nullptr );


  /* QWidget interface */
  GraphicsViewZoom* gvzoom( ) const;

protected:
  void mousePressEvent( QMouseEvent *e );
  void mouseReleaseEvent( QMouseEvent *e );
  void mouseMoveEvent( QMouseEvent *e );
  void keyPressEvent( QKeyEvent *e );
  void keyReleaseEvent( QKeyEvent *e );

private:
  bool _pan;
  bool _space;
  int _panStartX;
  int _panStartY;

  GraphicsViewZoom *m_gvzoom;

};

#endif /* GRAPHICSVIEW_H */
