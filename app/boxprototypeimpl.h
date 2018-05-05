#ifndef BOXPROTOTYPEIMPL_H
#define BOXPROTOTYPEIMPL_H
#include <QVector>

class GraphicElement;
class QGraphicsItem;
class QNEPort;


class BoxPrototypeImpl {

public:

  QVector< GraphicElement* > elements;
  QVector< QString > inputLabels;
  QVector< QString > outputLabels;

  QVector< QNEPort* > inputs;
  QVector< QNEPort* > outputs;

  ~BoxPrototypeImpl( );
  void loadFile( QString fileName );
  void clear( );

  int getInputSize( ) const;
  int getOutputSize( ) const;
  void setOutputSize( int outSize );
  void setInputSize( int inSize );

  void loadInputElement( GraphicElement *elm );

  void loadOutputElement( GraphicElement *elm );

private:

  void sortPorts( QVector< QNEPort* > &map );
  void loadItem( QGraphicsItem *item );
  void loadInputs( );
  void loadOutputs( );
};

#endif // BOXPROTOTYPEIMPL_H
