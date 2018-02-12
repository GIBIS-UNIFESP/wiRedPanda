#ifndef BOXPROTOTYPEIMPL_H
#define BOXPROTOTYPEIMPL_H

#include <QVector>

class GraphicElement;
class QGraphicsItem;
class QNEPort;


class BoxPrototypeImpl {

public:

  QVector< GraphicElement* > elements;
  QVector< QNEPort* > inputMap;
  QVector< QNEPort* > outputMap;
  QVector< bool > defaultInputValues;
  QVector< bool > requiredInputs;
  QVector< QString > inputLabels;
  QVector< QString > outputLabels;

  void loadFile( QString fileName );
  void clear( );

  int getInputSize( ) const;
  int getOutputSize( ) const;
  void setOutputSize( int outSize );
  void setInputSize( int inSize );

private:

  void sortMap( QVector< QNEPort* > &map );
  void loadItem( QGraphicsItem *item );
  void loadInputs( );
  void loadOutputs( );
};

#endif // BOXPROTOTYPEIMPL_H
