#ifndef BOXPROTOTYPEIMPL_H
#define BOXPROTOTYPEIMPL_H
#include <QVector>

class GraphicElement;
class QGraphicsItem;
class QNEPort;


class BoxPrototypeImpl {
  QVector< QNEPort* > referenceInputs;

public:

  QVector< GraphicElement* > elements;
  QVector< QNEPort* > referenceOutputs;
  QVector< bool > defaultInputValues;
  QVector< bool > requiredInputs;
  QVector< QString > inputLabels;
  QVector< QString > outputLabels;

  QVector< GraphicElement* > inputs;
  QVector< GraphicElement* > outputs;


  ~BoxPrototypeImpl( );
  void loadFile( QString fileName );
  void clear( );

  int getInputSize( ) const;
  int getOutputSize( ) const;
  void setOutputSize( int outSize );
  void setInputSize( int inSize );

  void loadInput( GraphicElement *elm );

  void loadOutput( GraphicElement *elm );

private:

  void sortMap( QVector< QNEPort* > &map );
  void loadItem( QGraphicsItem *item );
  void loadInputs( );
  void loadOutputs( );
};

#endif // BOXPROTOTYPEIMPL_H
