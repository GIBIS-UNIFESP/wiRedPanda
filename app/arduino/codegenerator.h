#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QFile>
#include <QTextStream>
#include <graphicelement.h>

class MappedPin {
public:
  GraphicElement *elm;
  QString pin;
  QString varName;
  QNEPort *port;
  int portNbr;
  MappedPin( GraphicElement *elm, QString pin, QString varName, QNEPort *port, int portNbr = 0 ) :
    elm( elm ), pin( pin ), varName( varName ), port( port ), portNbr( portNbr ) {
  }

  MappedPin( ) {

  }
};

class CodeGenerator {
public:
  CodeGenerator( QString fileName, const QVector< GraphicElement* > &elements );
  bool generate( );
  void declareAuxVariables();
  
private:
  void declareInputs( );
  void declareOutputs( );

  QFile file;
  QTextStream out;
  const QVector< GraphicElement* > elements;
  QVector< MappedPin > inputMap, outputMap;
  QMap< QNEPort*, QString > varMap;
  QVector< QString > availblePins;
  void setup( );
  void loop( );
  unsigned int globalCounter = 1;
};

#endif /* CODEGENERATOR_H */
