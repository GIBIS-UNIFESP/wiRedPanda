#include "testelements.h"
#include <QDebug>
#include <array>
#include <iostream>

#include "and.h"
#include "dflipflop.h"
#include "inputgnd.h"
#include "inputvcc.h"
#include "or.h"

TestElements::TestElements( QObject *parent ) : QObject( parent ) {

}

void TestElements::init( ) {
  /* Creating connections */
  for( size_t i = 0; i < 4; ++i ) {
    conn[ i ] = new QNEConnection( );
    sw[ i ] = new InputSwitch( );
    conn[ i ]->setPort1( sw[ i ]->outputs( ).first( ) );
  }
}


void TestElements::cleanup( ) {
  for( size_t i = 0; i < 4; ++i ) {
    delete sw[ i ];
  }
}


void TestElements::testAnd( ) {
  And elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 2 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 1 );
  QCOMPARE( elm.minInputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::AND );
  conn[ 0 ]->setPort2( elm.inputs( ).at( 0 ) );
  conn[ 1 ]->setPort2( elm.inputs( ).at( 1 ) );

  int truthTable[ 4 ][ 3 ] = {
    { 1, 1, 1 },
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 0 }
  };
  for( int test = 0; test < 4; ++test ) {
    sw[ 0 ]->setOn( truthTable[ test ][ 0 ] );
    sw[ 1 ]->setOn( truthTable[ test ][ 1 ] );
    sw[ 0 ]->updateLogic( );
    sw[ 1 ]->updateLogic( );
    elm.updateLogic( );
    QCOMPARE( ( int ) elm.outputs( ).front( )->value( ), truthTable[ test ][ 2 ] );
  }
}

void TestElements::testOr( ) {
  Or elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 2 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 1 );
  QCOMPARE( elm.minInputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::OR );
  conn[ 0 ]->setPort2( elm.inputs( ).at( 0 ) );
  conn[ 1 ]->setPort2( elm.inputs( ).at( 1 ) );

  int truthTable[ 4 ][ 3 ] = {
    { 1, 1, 1 },
    { 1, 0, 1 },
    { 0, 1, 1 },
    { 0, 0, 0 }
  };
  for( int test = 0; test < 4; ++test ) {
    sw[ 0 ]->setOn( truthTable[ test ][ 0 ] );
    sw[ 1 ]->setOn( truthTable[ test ][ 1 ] );
    sw[ 0 ]->updateLogic( );
    sw[ 1 ]->updateLogic( );
    elm.updateLogic( );
    QCOMPARE( ( int ) elm.outputs( ).front( )->value( ), truthTable[ test ][ 2 ] );
  }
}

void TestElements::testVCC( ) {
  InputVcc vcc;
  QCOMPARE( ( int ) vcc.outputs( ).front( )->value( ), 1 );
}


void TestElements::testGND( ) {
  InputGnd gnd;
  QCOMPARE( ( int ) gnd.outputs( ).front( )->value( ), 0 );

}

void TestElements::testDFlipFlop( ) {
  DFlipFlop elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 4 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 2 );
  QCOMPARE( elm.minInputSz( ), 4 );
  QCOMPARE( elm.maxInputSz( ), 4 );
  QCOMPARE( elm.minOutputSz( ), 2 );
  QCOMPARE( elm.maxOutputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::DFLIPFLOP );
  conn[ 0 ]->setPort2( elm.inputs( ).at( 0 ) );
  conn[ 1 ]->setPort2( elm.inputs( ).at( 1 ) );
  conn[ 2 ]->setPort2( elm.inputs( ).at( 2 ) );
  conn[ 3 ]->setPort2( elm.inputs( ).at( 3 ) );

  std::array< std::array< int, 7 >, 7 > truthTable = {
    {
      /* L  D  C  p  c  Q ~Q */
      { 0, 0, 1, 1, 1, 0, 1 }, /* Clk up and D = 0 */
      { 0, 1, 1, 1, 1, 1, 0 }, /* Clk up and D = 1 */

      { 0, 0, 0, 0, 1, 1, 0 }, /* Preset = false */
      { 0, 0, 1, 1, 0, 0, 1 }, /* Clear = false */
      { 0, 0, 1, 0, 0, 1, 1 }, /* Clear and Preset = false */

      { 1, 0, 0, 1, 1, 1, 1 }, /* Clk dwn and D = 0 ( must mantain current state )*/
      { 1, 1, 0, 1, 1, 1, 1 }, /* Clk dwn and D = 1 ( must mantain current state )*/
    }
  };
  for( size_t test = 0; test < truthTable.size( ); ++test ) {
    sw[ 0 ]->setOn( false );
    sw[ 1 ]->setOn( truthTable[ test ][ 0 ] );
    sw[ 2 ]->setOn( false );
    sw[ 3 ]->setOn( false );
    for( int port = 0; port < 4; ++port ) {
      sw[ port ]->updateLogic( );
    }
    elm.updateLogic( );
    for( int port = 0; port < 4; ++port ) {
      sw[ port ]->setOn( truthTable[ test ][ port + 1 ] );
      sw[ port ]->updateLogic( );
      std::cout << truthTable[ test ][ port + 1 ] << " ";
    }
    elm.updateLogic( );
    std::cout << "-> "
              << ( int ) elm.outputs( ).at( 0 )->value( ) << " "
              << ( int ) elm.outputs( ).at( 1 )->value( ) << std::endl;
    QCOMPARE( ( int ) elm.outputs( ).at( 0 )->value( ), truthTable[ test ][ 5 ] );
    QCOMPARE( ( int ) elm.outputs( ).at( 1 )->value( ), truthTable[ test ][ 6 ] );
  }
  sw[ 2 ]->outputs( ).front( )->disconnect( conn[ 2 ] );
  elm.inputs( ).at( 2 )->disconnect( conn[ 2 ] );
  elm.updateLogic( );
  QVERIFY( ( int ) elm.outputs( ).at( 0 )->value( ) != -1 );
  QVERIFY( ( int ) elm.outputs( ).at( 1 )->value( ) != -1 );
}
