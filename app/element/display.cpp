#include "display.h"
#include "qneconnection.h"

#include <QDebug>
#include <QPainter>
#include <QPixmap>

Display::Display( QGraphicsItem *parent ) : GraphicElement( 8, 8, 0, 0, parent ) {
  setRotatable( false );
  setOutputsOnTop( true );
  updatePorts( );
  setBottomPosition( 58 );
  setTopPosition( 6 );
  setHasLabel( true );

  setPixmap( ":/output/counter/counter_off.png" );
  a = QPixmap( ":/output/counter/counter_a.png" );
  b = QPixmap( ":/output/counter/counter_b.png" );
  c = QPixmap( ":/output/counter/counter_c.png" );
  d = QPixmap( ":/output/counter/counter_d.png" );
  e = QPixmap( ":/output/counter/counter_e.png" );
  f = QPixmap( ":/output/counter/counter_f.png" );
  g = QPixmap( ":/output/counter/counter_g.png" );
  h = QPixmap( ":/output/counter/counter_h.png" );

  setPortName( "Display" );
  for( QNEPort *in : inputs( ) ) {
    in->setRequired( false );
    in->setDefaultValue( 0 );
  }
}

void Display::refresh( ) {
  update( );
}

void Display::updatePorts( ) {
  input( 0 )->setPos( topPosition( ), 10 ); /* G */
  input( 1 )->setPos( topPosition( ), 25 ); /* F */
  input( 2 )->setPos( topPosition( ), 39 ); /* E */
  input( 3 )->setPos( topPosition( ), 54 ); /* D */
  input( 4 )->setPos( bottomPosition( ), 10 ); /* A */
  input( 5 )->setPos( bottomPosition( ), 25 ); /* B */
  input( 6 )->setPos( bottomPosition( ), 39 ); /* DP */
  input( 7 )->setPos( bottomPosition( ), 54 ); /* C */
  input( 0 )->setName( "G (mid)" );
  input( 1 )->setName( "F (upper left)" );
  input( 2 )->setName( "E (lower left)" );
  input( 3 )->setName( "D (bottom)" );
  input( 4 )->setName( "A (top)" );
  input( 5 )->setName( "B (upper right)" );
  input( 6 )->setName( "DP (dot)" );
  input( 7 )->setName( "C (lower right)" );
}

void Display::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) {
  GraphicElement::paint( painter, option, widget );
  if( input( 0 )->value( ) == true ) { /* G */
    painter->drawPixmap( QPoint( 0, 0 ), g );
  }
  if( input( 1 )->value( ) == true ) { /* F */
    painter->drawPixmap( QPoint( 0, 0 ), f );
  }
  if( input( 2 )->value( ) == true ) { /* E */
    painter->drawPixmap( QPoint( 0, 0 ), e );
  }
  if( input( 3 )->value( ) == true ) { /* D */
    painter->drawPixmap( QPoint( 0, 0 ), d );
  }
  if( input( 4 )->value( ) == true ) { /* A */
    painter->drawPixmap( QPoint( 0, 0 ), a );
  }
  if( input( 5 )->value( ) == true ) { /* B */
    painter->drawPixmap( QPoint( 0, 0 ), b );
  }
  if( input( 6 )->value( ) == true ) { /* DP (H) */
    painter->drawPixmap( QPoint( 0, 0 ), h );
  }
  if( input( 7 )->value( ) == true ) { /* C */
    painter->drawPixmap( QPoint( 0, 0 ), c );
  }
}

void Display::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
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
}
