#ifndef BOXPROTOTYPE_H
#define BOXPROTOTYPE_H

#include "boxprototypeimpl.h"

#include <QGraphicsItem>
#include <QString>
#include <QVector>

class GraphicElement;
class Box;

class BoxPrototype {
  QString m_fileName;

  BoxPrototypeImpl boxImpl;
  QVector< Box* > boxObservers;

public:
  BoxPrototype( const QString &fileName );
  void reload( );

  QString fileName( ) const;
  QString baseName( ) const;

  void insertBoxObserver( Box *box );
  void removeBoxObserver( Box *box );

  int inputSize( );
  int outputSize( );

  QString inputLabel( int index );
  QString outputLabel( int index );

  bool defaultInputValue( int index );
  bool isInputRequired( int index );

private:
  void clear( );
};

#endif // BOXPROTOTYPE_H
