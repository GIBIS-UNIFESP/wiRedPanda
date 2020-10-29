#ifndef BOXPROTOTYPE_H
#define BOXPROTOTYPE_H

#include "boxprototypeimpl.h"

#include <QGraphicsItem>
#include <QString>
#include <QVector>

class BoxMapping;
class GraphicElement;
class Box;

class BoxPrototype {
  QString m_fileName;

  BoxPrototypeImpl boxImpl;
  QVector< Box* > boxObservers;

public:
  BoxPrototype( const QString& fileName );
  void reload( );

  QString fileName( ) const;
  QString baseName( ) const;

  void insertBoxObserver( Box *box );
  void removeBoxObserver( Box *box );

  int inputSize( ) const;
  int outputSize( ) const;

  QString inputLabel( int index ) const;
  QString outputLabel( int index ) const;

  bool defaultInputValue( int index );
  bool isInputRequired( int index );

  BoxMapping* generateMapping( ) const;

private:
  void clear( );
};

#endif // BOXPROTOTYPE_H