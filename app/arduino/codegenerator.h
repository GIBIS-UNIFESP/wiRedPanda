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
  int portNbr;

  MappedPin( GraphicElement *elm, QString pin, QString varName, int portNbr = 0 ) :
    elm( elm ), pin( pin ), varName( varName ), portNbr( portNbr ) {
  }

  MappedPin( ) {

  }
};

class CodeGenerator {
public:
  CodeGenerator( QString fileName, const QVector< GraphicElement* > &elements );
  bool generate( );
private:
  void declareInputs( );
  void declareOutputs( );

  QFile file;
  QTextStream out;
  const QVector< GraphicElement* > elements;
  QVector< MappedPin > inputMap, outputMap;

  QVector< QString > availblePins;
  void setup();
  void loop();
};

#endif /* CODEGENERATOR_H */
