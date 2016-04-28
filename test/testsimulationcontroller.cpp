#include "testsimulationcontroller.h"

#include <and.h>
#include <inputbutton.h>
#include <led.h>

void TestSimulationController::init( ) {
  editor = new Editor( this );
}

void TestSimulationController::cleanup( ) {
  editor->deleteLater( );
}

void TestSimulationController::testCase1( ) {
  InputButton *btn1 = new InputButton( );
  InputButton *btn2 = new InputButton( );
  And *andItem = new And( );
  Led *led = new Led( );
  QNEConnection *conn = new QNEConnection( );
  QNEConnection *conn2 = new QNEConnection( );
  QNEConnection *conn3 = new QNEConnection( );
  editor->getScene( )->addItem( led );
  editor->getScene( )->addItem( andItem );
  editor->getScene( )->addItem( btn1 );
  editor->getScene( )->addItem( btn2 );
  editor->getScene( )->addItem( conn );
  editor->getScene( )->addItem( conn2 );
  editor->getScene( )->addItem( conn3 );
  conn->setPort1( btn1->output( ) );
  conn->setPort2( andItem->input( 0 ) );
  conn2->setPort1( btn2->output( ) );
  conn2->setPort2( andItem->input( 1 ) );
  conn3->setPort1( andItem->output( ) );
  conn3->setPort2( led->input( ) );
  QVector< GraphicElement* > elms( SimulationController::sortElements( editor->getScene( )->getElements( ) ) );
  QVERIFY( elms.at( 0 ) == btn1 || elms.at( 1 ) == btn1 );
  QVERIFY( elms.at( 0 ) == btn2 || elms.at( 1 ) == btn2 );
  QVERIFY( elms.at( 2 ) == andItem );
  QVERIFY( elms.at( 3 ) == led );
}
