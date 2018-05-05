#include "testlogicelements.h"

#include <logicelement/logicand.h>
#include <logicelement/logicdemux.h>
#include <logicelement/logicdflipflop.h>
#include <logicelement/logicdlatch.h>
#include <logicelement/logicjkflipflop.h>
#include <logicelement/logicmux.h>
#include <logicelement/logicnand.h>
#include <logicelement/logicnode.h>
#include <logicelement/logicnor.h>
#include <logicelement/logicnot.h>
#include <logicelement/logicor.h>
#include <logicelement/logicoutput.h>
#include <logicelement/logicsrflipflop.h>
#include <logicelement/logictflipflop.h>
#include <logicelement/logicxnor.h>
#include <logicelement/logicxor.h>

TestLogicElements::TestLogicElements( QObject *parent ) : QObject( parent ) {
}

void TestLogicElements::init( ) {
  for( size_t i = 0; i < sw.size( ); ++i ) {
    sw[ i ] = new LogicInput( );
  }
}

void TestLogicElements::cleanup( ) {
  for( size_t i = 0; i < sw.size( ); ++i ) {
    delete sw[ i ];
  }
}

void TestLogicElements::testLogicNode( ) {
  LogicNode elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );

  std::array< std::array< int, 2 >, 2 > truthTable = {
    {
      { { 1, 1 } },
      { { 0, 0 } }
    }
  };
  for( size_t test = 0; test < truthTable.size( ); ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 0 ) );
    elm.updateLogic( );
    QCOMPARE( ( int ) elm.getOutputValue( ), truthTable.at( test ).at( 1 ) );
  }
}

void TestLogicElements::testLogicAnd( ) {
  LogicAnd elm( 2 );
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );

  std::array< std::array< int, 3 >, 4 > truthTable = {
    {
      { { 1, 1, 1 } },
      { { 1, 0, 0 } },
      { { 0, 1, 0 } },
      { { 0, 0, 0 } }
    }
  };
  for( int test = 0; test < 4; ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 0 ) );
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 1 ) );
    elm.updateLogic( );

    QCOMPARE( ( int ) elm.getOutputValue( ), truthTable.at( test ).at( 2 ) );
  }
}

void TestLogicElements::testLogicOr( ) {
  LogicOr elm( 2 );
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );

  std::array< std::array< int, 3 >, 4 > truthTable = {
    {
      { { 1, 1, 1 } },
      { { 1, 0, 1 } },
      { { 0, 1, 1 } },
      { { 0, 0, 0 } }
    }
  };
  for( int test = 0; test < 4; ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 0 ) );
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 1 ) );
    elm.updateLogic( );

    QCOMPARE( ( int ) elm.getOutputValue( ), truthTable.at( test ).at( 2 ) );
  }
}

void TestLogicElements::testLogicInput( ) {
  LogicInput elm;
  QCOMPARE( elm.getOutputValue( ), false );
  elm.setOutputValue( true );
  QCOMPARE( elm.getOutputValue( ), true );
  elm.setOutputValue( false );
  QCOMPARE( elm.getOutputValue( ), false );
}

void TestLogicElements::testLogicMux( ) {
  LogicMux elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );
  elm.connectPredecessor( 2, sw.at( 2 ), 0 );

  std::array< std::array< int, 4 >, 8 > truthTable = {
    {
      { { 0, 0, 0, 0 } },
      { { 0, 0, 1, 0 } },
      { { 0, 1, 0, 0 } },
      { { 0, 1, 1, 1 } },
      { { 1, 0, 0, 1 } },
      { { 1, 0, 1, 0 } },
      { { 1, 1, 0, 1 } },
      { { 1, 1, 1, 1 } },
    }
  };
  for( int test = 0; test < 4; ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 0 ) );
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 1 ) );
    sw.at( 2 )->setOutputValue( truthTable.at( test ).at( 2 ) );
    elm.updateLogic( );
    QCOMPARE( ( int ) elm.getOutputValue( ), truthTable.at( test ).at( 3 ) );
  }
}

void TestLogicElements::testLogicDemux( ) {
  LogicDemux elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );

  std::array< std::array< int, 4 >, 8 > truthTable = {
    {
      /*  i  S  o0 o1 */
      { { 0, 0, 0, 0 } },
      { { 0, 1, 0, 0 } },
      { { 1, 0, 1, 0 } },
      { { 1, 1, 0, 1 } },
    }
  };
  for( int test = 0; test < 4; ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 0 ) );
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 1 ) );
    elm.updateLogic( );
    QCOMPARE( ( int ) elm.getOutputValue( 0 ), truthTable.at( test ).at( 2 ) );
    QCOMPARE( ( int ) elm.getOutputValue( 1 ), truthTable.at( test ).at( 3 ) );
  }
}

void TestLogicElements::testLogicDFlipFlop( ) {
  LogicDFlipFlop elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );
  elm.connectPredecessor( 2, sw.at( 2 ), 0 );
  elm.connectPredecessor( 3, sw.at( 3 ), 0 );

  std::array< std::array< int, 7 >, 7 > truthTable = {
    {
      /*  L  D  C  p  c  Q ~Q */
      { { 0, 0, 1, 1, 1, 0, 1 } }, /* Clk up and D = 0 */
      { { 0, 1, 1, 1, 1, 1, 0 } }, /* Clk up and D = 1 */

      { { 0, 0, 0, 0, 1, 1, 0 } }, /* Preset = false */
      { { 0, 0, 1, 1, 0, 0, 1 } }, /* Clear = false */
      { { 0, 0, 1, 0, 0, 1, 1 } }, /* Clear and Preset = false */

      { { 1, 0, 0, 1, 1, 1, 0 } }, /* Clk dwn and D = 0 ( must mantain current state )*/
      { { 1, 1, 0, 1, 1, 1, 0 } }, /* Clk dwn and D = 1 ( must mantain current state )*/
    }
  };
  for( size_t test = 0; test < truthTable.size( ); ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 1 ) ); /* DATA */
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 0 ) ); /*  CLK */
    sw.at( 2 )->setOutputValue( false ); /* PRST */
    sw.at( 3 )->setOutputValue( false ); /* CLR */

    elm.updateLogic( );
    elm.setOutputValue( 0, truthTable.at( test ).at( 0 ) );
    elm.setOutputValue( 1, !truthTable.at( test ).at( 0 ) );
    for( int port = 0; port < 4; ++port ) {
      sw.at( port )->setOutputValue( truthTable.at( test ).at( port + 1 ) );
    }
    elm.updateLogic( );

    QCOMPARE( ( int ) elm.getOutputValue( 0 ), truthTable.at( test ).at( 5 ) );
    QCOMPARE( ( int ) elm.getOutputValue( 1 ), truthTable.at( test ).at( 6 ) );
  }
}

void TestLogicElements::testLogicDLatch( ) {
  LogicDLatch elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );

  std::array< std::array< int, 4 >, 6 > truthTable = {
    {
      /*D  E  Q  A */
      { { 0, 0, 0, 0 } },
      { { 0, 0, 1, 1 } },
      { { 1, 0, 0, 0 } },
      { { 1, 0, 1, 1 } },
      { { 0, 1, 0, 1 } },
      { { 1, 1, 1, 0 } },
    }
  };
  for( size_t test = 0; test < truthTable.size( ); ++test ) {
    elm.setOutputValue( 0, truthTable.at( test ).at( 3 ) );
    elm.setOutputValue( 1, !truthTable.at( test ).at( 3 ) );
    for( int port = 0; port < 2; ++port ) {
      sw.at( port )->setOutputValue( truthTable.at( test ).at( port ) );
    }
    elm.updateLogic( );
    QCOMPARE( ( int ) elm.getOutputValue( 0 ), truthTable.at( test ).at( 2 ) );
    QCOMPARE( ( int ) elm.getOutputValue( 1 ), ( int ) !truthTable.at( test ).at( 2 ) );
  }
}

void TestLogicElements::testLogicJKFlipFlop( ) {
  LogicJKFlipFlop elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );
  elm.connectPredecessor( 2, sw.at( 2 ), 0 );
  elm.connectPredecessor( 3, sw.at( 3 ), 0 );
  elm.connectPredecessor( 4, sw.at( 4 ), 0 );

  std::array< std::array< int, 9 >, 13 > truthTable = {
    {
      /*  L  J  C  K  p  c  Q  Q  A */
      { { 0, 0, 0, 0, 0, 1, 1, 0, 0 } }, /* Preset = false */
      { { 0, 0, 0, 0, 1, 0, 0, 1, 0 } }, /* Clear = false */
      { { 0, 0, 0, 1, 0, 0, 1, 1, 0 } }, /* Clear and Preset = false*/
      { { 1, 1, 0, 0, 1, 1, 0, 1, 0 } }, /* Clk dwn and J = 0 ( must mantain current state )*/
      { { 1, 1, 0, 0, 1, 1, 0, 1, 0 } }, /* Clk dwn and J = 1 ( must mantain current state )*/
      { { 0, 1, 1, 1, 1, 1, 1, 0, 0 } }, /* Clk up J = 1 K = 1 ( must swap Q and ~Q )*/
      { { 0, 1, 1, 1, 1, 1, 0, 1, 1 } }, /* Clk up J = 1 K = 1 ( must swap Q and ~Q )*/
      { { 0, 1, 1, 0, 1, 1, 1, 0, 0 } }, /* Clk up J = 1 K = 0 */
      { { 0, 1, 1, 0, 1, 1, 1, 0, 1 } }, /* Clk up J = 1 K = 0 */
      { { 0, 0, 1, 1, 1, 1, 0, 1, 0 } }, /* Clk up J = 0 K = 1 */
      { { 0, 0, 1, 1, 1, 1, 0, 1, 1 } }, /* Clk up J = 0 K = 1 */
      { { 0, 0, 1, 0, 1, 1, 0, 1, 0 } }, /* Clk up J = 0 K = 0 */
      { { 0, 0, 1, 0, 1, 1, 1, 0, 1 } }, /* Clk up J = 0 K = 0 */
    }
  };
  elm.updateLogic( );
  for( size_t test = 0; test < truthTable.size( ); ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 1 ) );
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 0 ) );
    sw.at( 2 )->setOutputValue( truthTable.at( test ).at( 3 ) );
    sw.at( 3 )->setOutputValue( false );
    sw.at( 4 )->setOutputValue( false );
    for( int port = 0; port < 5; ++port ) {

    }
    elm.updateLogic( );
    elm.setOutputValue( 0, truthTable.at( test ).at( 8 ) );
    elm.setOutputValue( 1, !truthTable.at( test ).at( 8 ) );
    for( int port = 0; port < 5; ++port ) {
      sw.at( port )->setOutputValue( truthTable.at( test ).at( port + 1 ) );

    }
    elm.updateLogic( );

    QCOMPARE( ( int ) elm.getOutputValue( 0 ), truthTable.at( test ).at( 6 ) );
    QCOMPARE( ( int ) elm.getOutputValue( 1 ), truthTable.at( test ).at( 7 ) );
  }
}


void TestLogicElements::testLogicSRFlipFlop( ) {
  LogicSRFlipFlop elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );
  elm.connectPredecessor( 2, sw.at( 2 ), 0 );
  elm.connectPredecessor( 3, sw.at( 3 ), 0 );
  elm.connectPredecessor( 4, sw.at( 4 ), 0 );


  std::array< std::array< int, 9 >, 23 > truthTable = {
    {
      /*  L  S  C  R  p  c  Q  Q  A */

      { { 0, 0, 0, 0, 0, 1, 1, 0, 0 } }, /* Preset = false */
      { { 0, 0, 0, 0, 1, 0, 0, 1, 1 } }, /* Clear = false*/
      { { 0, 0, 0, 0, 0, 0, 1, 1, 1 } }, /* Preset || Clear = false*/

      { { 0, 0, 0, 0, 1, 1, 0, 1, 0 } }, /* No change */
      { { 0, 0, 0, 0, 1, 1, 1, 0, 1 } }, /* No change */
      { { 0, 1, 0, 0, 1, 1, 0, 1, 0 } }, /* No change */
      { { 0, 1, 0, 0, 1, 1, 1, 0, 1 } }, /* No change */

      { { 0, 0, 0, 1, 1, 1, 0, 1, 0 } }, /* No change */
      { { 0, 0, 0, 1, 1, 1, 1, 0, 1 } }, /* No change */
      { { 1, 0, 1, 0, 1, 1, 0, 1, 0 } }, /* No change */
      { { 1, 0, 1, 0, 1, 1, 1, 0, 1 } }, /* No change */

      { { 1, 1, 1, 0, 1, 1, 0, 1, 0 } }, /* No change */
      { { 1, 1, 1, 0, 1, 1, 1, 0, 1 } }, /* No change */
      { { 1, 0, 1, 1, 1, 1, 0, 1, 0 } }, /* No change */
      { { 1, 0, 1, 1, 1, 1, 1, 0, 1 } }, /* No change */

      { { 0, 0, 1, 0, 1, 1, 0, 1, 0 } }, /* No change */
      { { 0, 0, 1, 0, 1, 1, 1, 0, 1 } }, /* No change */
      { { 0, 0, 1, 1, 1, 1, 0, 1, 0 } }, /* Q = 0 */
      { { 0, 0, 1, 1, 1, 1, 0, 1, 1 } }, /* Q = 0 */

      { { 0, 1, 1, 0, 1, 1, 1, 0, 0 } }, /* Q = 1 */
      { { 0, 1, 1, 0, 1, 1, 1, 0, 1 } }, /* Q = 1 */
      { { 0, 1, 1, 1, 1, 1, 1, 1, 0 } }, /* Not permitted */
      { { 0, 1, 1, 1, 1, 1, 1, 1, 1 } }, /* Not permitted */
    }
  };
  elm.updateLogic( );
  for( size_t test = 0; test < truthTable.size( ); ++test ) {
    sw.at( 0 )->setOutputValue( false );
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 0 ) );
    sw.at( 2 )->setOutputValue( false );
    elm.updateLogic( );
    elm.setOutputValue( 0, truthTable.at( test ).at( 8 ) );
    elm.setOutputValue( 1, !truthTable.at( test ).at( 8 ) );
    for( int port = 0; port < 5; ++port ) {
      sw.at( port )->setOutputValue( truthTable.at( test ).at( port + 1 ) );
    }
    elm.updateLogic( );
    QCOMPARE( ( int ) elm.getOutputValue( 0 ), truthTable.at( test ).at( 6 ) );
    QCOMPARE( ( int ) elm.getOutputValue( 1 ), truthTable.at( test ).at( 7 ) );
  }
}

void TestLogicElements::testLogicTFlipFlop( ) {
  LogicTFlipFlop elm;
  elm.connectPredecessor( 0, sw.at( 0 ), 0 );
  elm.connectPredecessor( 1, sw.at( 1 ), 0 );
  elm.connectPredecessor( 2, sw.at( 2 ), 0 );
  elm.connectPredecessor( 3, sw.at( 3 ), 0 );

  std::array< std::array< int, 8 >, 11 > truthTable = {
    {
      /*  L  T  C  p  c  Q ~Q  A */
      { { 1, 0, 1, 1, 1, 0, 1, 0 } }, /* No change */
      { { 1, 1, 1, 1, 1, 0, 1, 0 } }, /* No change */
      { { 1, 0, 1, 1, 1, 1, 0, 1 } }, /* No change */
      { { 1, 1, 1, 1, 1, 1, 0, 1 } }, /* No change */

      { { 1, 0, 0, 1, 1, 0, 1, 0 } }, /* No change */
      { { 1, 0, 0, 1, 1, 1, 0, 1 } }, /* No change */

      { { 0, 1, 1, 1, 1, 1, 0, 0 } }, /* Toggle */
      { { 0, 1, 1, 1, 1, 0, 1, 1 } }, /* Toggle */


      { { 1, 0, 1, 0, 1, 1, 0, 0 } }, /* Preset = false */
      { { 1, 0, 1, 1, 0, 0, 1, 1 } }, /* Clear = false */
      { { 1, 0, 1, 0, 0, 1, 1, 1 } }, /* Clear and Preset = false */

      /* Test Prst and clr */
    }
  };
  elm.updateLogic( );
  for( size_t test = 0; test < truthTable.size( ); ++test ) {
    sw.at( 0 )->setOutputValue( truthTable.at( test ).at( 1 ) ); // T
    sw.at( 1 )->setOutputValue( truthTable.at( test ).at( 0 ) ); // CLK
    sw.at( 2 )->setOutputValue( true );
    sw.at( 3 )->setOutputValue( true );
    for( int port = 0; port < 4; ++port ) {

    }
    elm.updateLogic( );
    elm.setOutputValue( 0, truthTable.at( test ).at( 7 ) );
    elm.setOutputValue( 1, !truthTable.at( test ).at( 7 ) );
    for( int port = 0; port < 4; ++port ) {
      sw.at( port )->setOutputValue( truthTable.at( test ).at( port + 1 ) );
    }
    elm.updateLogic( );

    QCOMPARE( ( int ) elm.getOutputValue( 0 ), truthTable.at( test ).at( 5 ) );
    QCOMPARE( ( int ) elm.getOutputValue( 1 ), truthTable.at( test ).at( 6 ) );
  }
}
