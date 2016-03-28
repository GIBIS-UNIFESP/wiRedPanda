#ifndef INPUTBUTTON_H
#define INPUTBUTTON_H

#include "graphicelement.h"
#include "input.h"

class InputButton : public GraphicElement, public Input {
public:
  explicit InputButton( QGraphicsItem *parent = 0);
  virtual ~InputButton( );
  bool on;

  /* QGraphicsItem interface */
protected:
  void mousePressEvent( QGraphicsSceneMouseEvent *event );
  void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::BUTTON );
  }
  void updateLogic( );

  // Input interface
public:
  bool getOn() const;
  void setOn(bool value);
};

#endif /* INPUTBUTTON_H */
