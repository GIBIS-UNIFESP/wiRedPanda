#ifndef BOXPROTOTYPE_H
#define BOXPROTOTYPE_H

#include "boxprototypeimpl.h"

#include <QGraphicsItem>
#include <QString>
#include <QVector>

class GraphicElement;

class BoxPrototype {
  QString m_fileName;

  BoxPrototypeImpl boxImpl;

public:
  BoxPrototype( const QString &fileName );
  void reload( );

  QString fileName( ) const;
  int inputSize( ) const;
  int outputSize( ) const;

private:
  void clear( );
};

#endif // BOXPROTOTYPE_H
