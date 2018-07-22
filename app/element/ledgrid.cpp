#include "ledgrid.h"
#include <array>
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
  setPortName( "Led Grid" );
  for( QNEPort *in : inputs( ) ) {
    in->setRequired( false );
    in->setDefaultValue( 0 );
  }
  m_rowOffsets = {{ 8, 76, 144, 212 } };
  m_colOffsets = {{ 15, 85, 155, 225 } };

  refresh( );
}

LedGrid::~LedGrid( ) {

}

void LedGrid::refresh( ) {
  update( );
}

void LedGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) {
  GraphicElement::paint( painter, option, widget );
  bool outputMatrix[ 4 ][ 4 ];
  for( int i = 0; i < 4; ++i ) {
    for( int j = 0; j < 4; ++j ) {
      outputMatrix[ i ][ j ] = false;
    }
    if( input( i )->value( ) == true ) {
      for( int j = 0; j < 4; ++j ) {
        if( input( 7 - j )->value( ) == false ) {
          outputMatrix[ i ][ j ] = true;
        }
      }
    }
  }
  for( int i = 0; i < 4; ++i ) {
    for( int j = 0; j < 4; ++j ) {
      if( outputMatrix[ i ][ j ] == false ) {
        painter->drawPixmap( QPoint( m_colOffsets[ j ], m_rowOffsets[ i ] ), a );
      }
      else {
        painter->drawPixmap( QPoint( m_colOffsets[ j ], m_rowOffsets[ i ] ), b );
      }
    }
  }
}

void LedGrid::setColor( QString color ) {
  m_color = color;
  a = QPixmap( ":/output/" + getColor( ) + "LedOff.png" );
  b = QPixmap( ":/output/" + getColor( ) + "LedOn.png" );
  refresh( );
}

QString LedGrid::getColor( ) const {
  return( m_color );
}

void LedGrid::save( QDataStream &ds ) const {
  GraphicElement::save( ds );
  ds << getColor( );
  qDebug( ) << "Getcolor" << getColor( );
}

void LedGrid::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
  QString clr;
  ds >> clr;
  qDebug( ) << clr;
  setColor( clr );
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
  input( 0 )->setName( "Row 1" );
  input( 1 )->setName( "Row 2" );
  input( 2 )->setName( "Row 3" );
  input( 3 )->setName( "Row 4" );
  input( 4 )->setName( "Col 1" );
  input( 5 )->setName( "Col 2" );
  input( 6 )->setName( "Col 3" );
  input( 7 )->setName( "Col 4" );
}
