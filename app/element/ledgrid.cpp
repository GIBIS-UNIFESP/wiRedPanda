#include "ledgrid.h"
#include <qneconnection.h>
#include <QDebug>
#include <QPainter>
#include <QPixmap>

int LedGrid::current_id_number = 0;

LedGrid::LedGrid( QGraphicsItem *parent ) : GraphicElement( 8, 8, 0, 0, parent ) {
  pixmapSkinName.append( ":/output/LedGrid.png" ); // 0
  pixmapSkinName.append( ":/output/WhiteLedOff.png" ); // 1
  pixmapSkinName.append( ":/output/WhiteLedOn.png" ); // 2
  pixmapSkinName.append( ":/output/RedLedOff.png" ); // 3
  pixmapSkinName.append( ":/output/RedLedOn.png" ); // 4
  pixmapSkinName.append( ":/output/GreenLedOff.png" ); // 5
  pixmapSkinName.append( ":/output/GreenLedOn.png" ); // 6
  pixmapSkinName.append( ":/output/BlueLedOff.png" ); // 7
  pixmapSkinName.append( ":/output/BlueLedOn.png" ); // 8
  pixmapSkinName.append( ":/output/PurpleLedOff.png" ); // 9
  pixmapSkinName.append( ":/output/PurpleLedOn.png" ); // 10
  setRotatable( false );
  setOutputsOnTop( true );
  updatePorts( );
  setHasColors( true );
  setColor( "White" );
  updatePorts( );
  setHasLabel( true );

  setPixmap( pixmapSkinName[ 0 ] );
  setPortName( "Led Grid" );
  for( QNEPort *in : qAsConst(m_inputs) ) {
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
        painter->drawPixmap( QPoint( m_colOffsets[ j ], m_rowOffsets[ i ] ), led_off_pixmap );
      }
      else {
        painter->drawPixmap( QPoint( m_colOffsets[ j ], m_rowOffsets[ i ] ), led_on_pixmap );
      }
    }
  }
}

void LedGrid::setColor( QString color ) {
  m_color = color;
  if( color == "White" )
    m_colorNumber = 0;
  else if( color == "Red" )
    m_colorNumber = 2;
  else if( color == "Green" )
    m_colorNumber = 4;
  else if( color == "Blue" )
    m_colorNumber = 6;
  else if( color == "Purple" )
    m_colorNumber = 8;
  led_off_pixmap = QPixmap( pixmapSkinName[ 1 + m_colorNumber ] );
  led_on_pixmap = QPixmap( pixmapSkinName[ 2 + m_colorNumber ] );
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

void LedGrid::setSkin( bool defaultSkin, QString filename ) {
}

