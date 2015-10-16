#include "dlatch.h"

DLatch::DLatch( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/D-latch.png" ) );
  setRotatable( false );
  updatePorts( );
  setObjectName( "D Latch" );
  inputs( ).at( 0 )->setName( "Data" );
  inputs( ).at( 1 )->setName( "Enable" );
  outputs( ).at( 0 )->setName( "Q" );
  outputs( ).at( 1 )->setName( "~Q" );
}

DLatch::~DLatch( ) {

}

void DLatch::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* Data */
  inputs( ).at( 1 )->setPos( topPosition( ), 45 ); /* Enable */

  outputs( ).at( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  outputs( ).at( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void DLatch::updateLogic( ) {
  char res1 = outputs( ).at( 0 )->value( ); /* Output 1 */
  char res2 = outputs( ).at( 1 )->value( ); /* Output 1 */
  if( !isValid( ) ) {
    res1 = res2 = -1;
  }
  else {
    if( inputs( ).at( 1 )->value( ) == true ) { /* If Enabled */
      res1 = inputs( ).at( 0 )->value( ); /* Output = Data */
      res2 = !inputs( ).at( 0 )->value( ); /* Output = Data */
    }
  }
  outputs( ).at( 0 )->setValue( res1 );
  outputs( ).at( 1 )->setValue( res2 );
}
