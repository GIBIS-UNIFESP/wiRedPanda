#ifndef INPUT_H
#define INPUT_H


class Input {
public:
  virtual bool getOn( ) const = 0;
  virtual void setOn( bool value ) = 0;

};

#endif /* INPUT_H */
