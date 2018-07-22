#include "ledgrid.h"
#include <qneconnection.h>
#include <QDebug>
#include <QPainter>
#include <QPixmap>

LedGrid::LedGrid( QGraphicsItem *parent ) : GraphicElement( 8, 8, 0, 0, parent ) {
  setRotatable( false );
  setOutputsOnTop( true );
  updatePorts( );
  setHasColors( true );
  setColor( "White" );
  updatePorts( );
  setHasLabel( true );

  setPixmap( ":/output/LedGrid.png" );
  a = QPixmap( ":/output/" + getColor( ) + "LedOff.png" );
  b = QPixmap( ":/output/" + getColor( ) + "LedOn.png" );
  setPortName( "Led Grid" );
  for( QNEPort *in : inputs( ) ) {
    in->setRequired( false );
    in->setDefaultValue( 0 );
  }
}

LedGrid::~LedGrid( ) {

}

void LedGrid::refresh( ) {
  a = QPixmap( ":/output/" + getColor( ) + "LedOff.png" );
  b = QPixmap( ":/output/" + getColor( ) + "LedOn.png" );
  update( );
}



void LedGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) {
  GraphicElement::paint( painter, option, widget );
  if( input( 0 )->value( ) == false ) {
    painter->drawPixmap( QPoint( 15, 8 ), a );
    painter->drawPixmap( QPoint( 85, 8 ), a );
    painter->drawPixmap( QPoint( 155, 8 ), a );
    painter->drawPixmap( QPoint( 225, 8 ), a );
  }
  else {
    if( input( 7 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 15, 8 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 15, 8 ), a );
    }
    if( input( 6 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 85, 8 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 85, 8 ), a );
    }
    if( input( 5 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 155, 8 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 155, 8 ), a );
    }
    if( input( 4 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 225, 8 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 225, 8 ), a );
    }
  }
  if( input( 1 )->value( ) == false ) {
    painter->drawPixmap( QPoint( 15, 76 ), a );
    painter->drawPixmap( QPoint( 85, 76 ), a );
    painter->drawPixmap( QPoint( 155, 76 ), a );
    painter->drawPixmap( QPoint( 225, 76 ), a );
  }
  else {
    if( input( 7 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 15, 76 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 15, 76 ), a );
    }
    if( input( 6 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 85, 76 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 85, 76 ), a );
    }
    if( input( 5 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 155, 76 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 155, 76 ), a );
    }
    if( input( 4 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 225, 76 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 225, 76 ), a );
    }
  }
  if( input( 2 )->value( ) == false ) {
    painter->drawPixmap( QPoint( 15, 144 ), a );
    painter->drawPixmap( QPoint( 85, 144 ), a );
    painter->drawPixmap( QPoint( 155, 144 ), a );
    painter->drawPixmap( QPoint( 225, 144 ), a );
  }
  else {
    if( input( 7 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 15, 144 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 15, 144 ), a );
    }
    if( input( 6 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 85, 144 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 85, 144 ), a );
    }
    if( input( 5 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 155, 144 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 155, 144 ), a );
    }
    if( input( 4 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 225, 144 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 225, 144 ), a );
    }
  }
  if( input( 3 )->value( ) == false ) {
    painter->drawPixmap( QPoint( 15, 212 ), a );
    painter->drawPixmap( QPoint( 85, 212 ), a );
    painter->drawPixmap( QPoint( 155, 212 ), a );
    painter->drawPixmap( QPoint( 225, 212 ), a );
  }
  else {
    if( input( 7 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 15, 212 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 15, 212 ), a );
    }
    if( input( 6 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 85, 212 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 85, 212 ), a );
    }
    if( input( 5 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 155, 212 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 155, 212 ), a );
    }
    if( input( 4 )->value( ) == false ) {
      painter->drawPixmap( QPoint( 225, 212 ), b );
    }
    else {
      painter->drawPixmap( QPoint( 225, 212 ), a );
    }
  }
}
void LedGrid::setColor( QString color ) {
  m_color = color;
  refresh( );
}

QString LedGrid::getColor( ) {
  return( m_color );
}

void LedGrid::save( QDataStream &ds ) {
  GraphicElement::save( ds );
  ds << getColor( );
}

void LedGrid::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
//  qDebug( ) << "Version: " << version;
/*
 * 0,7,2,1,3,4,5,6
 * 7,5,4,2,1,4,6,3,0
 * 4,5,7,3,2,1,0,6
 * 2,1,4,5,0,7,3,6
 */
  if( version < 1.6 ) {
    COMMENT( "Remapping inputs", 0 );
    QVector< int > order = { 2, 1, 4, 5, 0, 7, 3, 6 };
    QVector< QNEInputPort* > aux = inputs( );
    for( int i = 0; i < aux.size( ); ++i ) {
      aux[ order[ i ] ] = inputs( )[ i ];
    }
    setInputs( aux );
    updatePorts( );
  }
  if( version < 1.7 ) {
    COMMENT( "Remapping inputs", 0 );
    QVector< int > order = { 2, 5, 4, 0, 7, 3, 6, 1 };
    QVector< QNEInputPort* > aux = inputs( );
    for( int i = 0; i < aux.size( ); ++i ) {
      aux[ order[ i ] ] = inputs( )[ i ];
    }
    setInputs( aux );
    updatePorts( );
  }
  if( version >= 1.1 ) {
    QString clr;
    ds >> clr;
    setColor( clr );
  }
}

QString LedGrid::genericProperties( ) {
  return( getColor( ) );
}

void LedGrid::updatePorts( ) {
  input( 0 )->setPos( 2, 40 ); /* G */
  input( 1 )->setPos( 2, 110 ); /* F */
  input( 2 )->setPos( 2, 180 ); /* E */
  input( 3 )->setPos( 2, 250 ); /* D */
  input( 4 )->setPos( 300, 40 ); /* A */
  input( 5 )->setPos( 300, 110 ); /* B */
  input( 6 )->setPos( 300, 180 ); /* DP */
  input( 7 )->setPos( 300, 250 ); /* C */
  input( 0 )->setName( "G (mid)" );
  input( 1 )->setName( "F (upper left)" );
  input( 2 )->setName( "E (lower left)" );
  input( 3 )->setName( "D (bottom)" );
  input( 4 )->setName( "A (top)" );
  input( 5 )->setName( "B (upper right)" );
  input( 6 )->setName( "DP (dot)" );
  input( 7 )->setName( "C (lower right)" );
}
