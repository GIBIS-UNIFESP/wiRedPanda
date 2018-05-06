#ifndef INPUTBUTTON_H
#define INPUTBUTTON_H

#include "graphicelement.h"
#include "input.h"

class InputButton : public GraphicElement, public Input {
public:
  explicit InputButton( QGraphicsItem *parent = nullptr);
  virtual ~InputButton( ) override;
  bool on;

  /* QGraphicsItem interface */
protected:
  void mousePressEvent( QGraphicsSceneMouseEvent *event ) override;
  void mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::BUTTON );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::INPUT );
  }
  void updateLogic( );

  // Input interface
public:
  bool getOn() const override;
  void setOn(bool value) override;
};

#endif /* INPUTBUTTON_H */
