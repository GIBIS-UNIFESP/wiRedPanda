#ifndef BOXPROTOTYPEIMPL_H
#define BOXPROTOTYPEIMPL_H

#include <QVector>

class GraphicElement;
class QGraphicsItem;
class QNEPort;


class BoxPrototypeImpl {

public:

  QVector< GraphicElement* > m_elements;
  QVector< QNEPort* > m_inputMap;
  QVector< QNEPort* > m_outputMap;
  QVector< bool > m_defaultInputValues;
  QVector< bool > m_requiredInputs;
  QVector< QString > m_inputLabels;
  QVector< QString > m_outputLabels;

  int m_outputSize;
  int m_inputSize;

  void loadFile( QString fileName );
  void clear( );

  int inputSize( ) const;
  int outputSize( ) const;
  void setOutputSize( int outputSize );
  void setInputSize( int inputSize );

private:
  void sortMap( QVector< QNEPort* > &map );
  void loadItem( QGraphicsItem *item );
  void loadInputs( );
  void loadOutputs( );
};

#endif // BOXPROTOTYPEIMPL_H
