#include "testelements.h"
#include <QDebug>

#include <box.h>
#include <demux.h>
#include <dlatch.h>
#include <editor.h>
#include <inputbutton.h>
#include <iostream>
#include <jkflipflop.h>
#include <jklatch.h>
#include <led.h>
#include <mux.h>
#include <node.h>
#include <srflipflop.h>
#include <tflipflop.h>
#include <tlatch.h>

#include "and.h"
#include "dflipflop.h"
#include "inputgnd.h"
#include "inputvcc.h"
#include "or.h"
#include <iostream>

TestElements::TestElements( QObject *parent ) : QObject( parent ) {

}

void TestElements::init( ) {
  /* Creating connections */
  for( size_t i = 0; i < conn.size( ); ++i ) {
    conn.at( i ) = new QNEConnection( );
    sw.at( i ) = new InputSwitch( );
    conn.at( i )->setStart( sw.at( i )->output( ) );
  }
}


void TestElements::cleanup( ) {
  for( size_t i = 0; i < conn.size( ); ++i ) {
    delete sw.at( i );
  }
}

void TestElements::testNode( ) {
  Node elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 1 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 1 );
  QCOMPARE( elm.minInputSz( ), 1 );
  QCOMPARE( elm.elementType( ), ElementType::NODE );
}


void TestElements::testAnd( ) {
  And elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 2 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 1 );
  QCOMPARE( elm.minInputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::AND );
}

void TestElements::testOr( ) {
  Or elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 2 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 1 );
  QCOMPARE( elm.minInputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::OR );
}

void TestElements::testVCC( ) {
  InputVcc vcc;
  QCOMPARE( vcc.outputSize( ), 1 );
  QCOMPARE( vcc.inputSize( ), 0 );
  QCOMPARE( ( int ) vcc.output( )->value( ), 1 );
}


void TestElements::testGND( ) {
  InputGnd gnd;
  QCOMPARE( gnd.outputSize( ), 1 );
  QCOMPARE( gnd.inputSize( ), 0 );
  QCOMPARE( ( int ) gnd.output( )->value( ), 0 );
}

void TestElements::testMux( ) {
  Mux elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 3 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 1 );
  QCOMPARE( elm.minInputSz( ), 3 );
  QCOMPARE( elm.elementType( ), ElementType::MUX );
}

void TestElements::testDemux( ) {
  Demux elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 2 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 2 );
  QCOMPARE( elm.minInputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::DEMUX );
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
}

void TestElements::testDLatch( ) {
  DLatch elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 2 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 2 );
  QCOMPARE( elm.minInputSz( ), 2 );
  QCOMPARE( elm.maxInputSz( ), 2 );
  QCOMPARE( elm.minOutputSz( ), 2 );
  QCOMPARE( elm.maxOutputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::DLATCH );
}

void TestElements::testJKFlipFlop( ) {
  JKFlipFlop elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 5 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 2 );
  QCOMPARE( elm.minInputSz( ), 5 );
  QCOMPARE( elm.maxInputSz( ), 5 );
  QCOMPARE( elm.minOutputSz( ), 2 );
  QCOMPARE( elm.maxOutputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::JKFLIPFLOP );
}

void TestElements::testSRFlipFlop( ) {
  SRFlipFlop elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 5 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 2 );
  QCOMPARE( elm.minInputSz( ), 5 );
  QCOMPARE( elm.maxInputSz( ), 5 );
  QCOMPARE( elm.minOutputSz( ), 2 );
  QCOMPARE( elm.maxOutputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::SRFLIPFLOP );
}

void TestElements::testTFlipFlop( ) {
  TFlipFlop elm;
  QCOMPARE( elm.inputSize( ), elm.inputs( ).size( ) );
  QCOMPARE( elm.inputSize( ), 4 );
  QCOMPARE( elm.outputSize( ), elm.outputs( ).size( ) );
  QCOMPARE( elm.outputSize( ), 2 );
  QCOMPARE( elm.minInputSz( ), 4 );
  QCOMPARE( elm.maxInputSz( ), 4 );
  QCOMPARE( elm.minOutputSz( ), 2 );
  QCOMPARE( elm.maxOutputSz( ), 2 );
  QCOMPARE( elm.elementType( ), ElementType::TFLIPFLOP );

}

QString testFile( QString fname ) {
  return( QString( "%1/../examples/%2" ).arg( CURRENTDIR, fname ) );
}

void TestElements::testBoxData( Box *box ) {
  QCOMPARE( ( int ) box->inputSize( ), 5 );
  QCOMPARE( ( int ) box->outputSize( ), 2 );


  QCOMPARE( box->input( 0 )->isRequired( ), false );
  QCOMPARE( box->input( 1 )->isRequired( ), false );
  QCOMPARE( box->input( 2 )->isRequired( ), true );
  QCOMPARE( box->input( 3 )->isRequired( ), false );
  QCOMPARE( box->input( 4 )->isRequired( ), false );

  QCOMPARE( ( int ) box->input( 0 )->value( ), 1 );
  QCOMPARE( ( int ) box->input( 1 )->value( ), 1 );
  QCOMPARE( ( int ) box->input( 2 )->value( ), -1 );
  QCOMPARE( ( int ) box->input( 3 )->value( ), 1 );
  QCOMPARE( ( int ) box->input( 4 )->value( ), 1 );
}

void TestElements::testBox( ) {
  BoxManager manager;
  QString boxFile = testFile( "jkflipflop.panda" );
  Scene scene;

  Box *box = new Box( );
  manager.loadBox( box, boxFile );

  testBoxData( box );

  InputButton *clkButton = new InputButton( );

  InputButton *prstButton = new InputButton( );

  Led *led = new Led( );
  Led *led2 = new Led( );

  QNEConnection *conn = new QNEConnection( );
  conn->setStart( clkButton->output( ) );
  conn->setEnd( box->input( 2 ) );

  QNEConnection *conn2 = new QNEConnection( );
  conn2->setStart( box->output( 0 ) );
  conn2->setEnd( led->input( ) );

  QNEConnection *conn3 = new QNEConnection( );
  conn3->setStart( prstButton->output( ) );
  conn3->setEnd( box->input( 0 ) );

  QNEConnection *conn4 = new QNEConnection( );
  conn4->setStart( box->output( 1 ) );
  conn4->setEnd( led2->input( ) );

  scene.addItem( led );
  scene.addItem( led2 );
  scene.addItem( clkButton );
  scene.addItem( prstButton );
  scene.addItem( box );
  scene.addItem( conn );
  scene.addItem( conn2 );
  scene.addItem( conn3 );
  scene.addItem( conn4 );

  SimulationController sc( &scene );
  sc.reSortElms( );
  for( int i = 0; i < 10; ++i ) {
    clkButton->setOn( false );
    prstButton->setOn( false );
    sc.update( );
    sc.update( );
    sc.update( );
    sc.updateScene( scene.itemsBoundingRect( ) );

    QCOMPARE( ( int ) box->input( 2 )->value( ), 0 );

    QCOMPARE( ( int ) box->output( 0 )->value( ), 1 );
    QCOMPARE( ( int ) box->output( 1 )->value( ), 0 );

    clkButton->setOn( false );
    prstButton->setOn( true );
    sc.update( );
    sc.update( );
    sc.update( );
    sc.updateScene( scene.itemsBoundingRect( ) );
    QCOMPARE( ( int ) box->input( 2 )->value( ), 0 );

    QCOMPARE( ( int ) box->output( 0 )->value( ), 1 );
    QCOMPARE( ( int ) box->output( 1 )->value( ), 0 );


    clkButton->setOn( false );
    sc.update( );
    sc.update( );
    sc.update( );
    sc.updateScene( scene.itemsBoundingRect( ) );

    QCOMPARE( ( int ) box->input( 2 )->value( ), 0 );

    QCOMPARE( ( int ) box->output( 0 )->value( ), 1 );
    QCOMPARE( ( int ) box->output( 1 )->value( ), 0 );


    clkButton->setOn( true );
    sc.update( );
    sc.update( );
    sc.update( );
    sc.updateScene( scene.itemsBoundingRect( ) );

    QCOMPARE( ( int ) box->input( 2 )->value( ), 1 );

    std::cout << ( int ) box->output( 0 )->value( ) << " " << ( int ) box->output( 1 )->value( ) << std::endl;

    QCOMPARE( ( int ) box->output( 0 )->value( ), 0 );
    QCOMPARE( ( int ) box->output( 1 )->value( ), 1 );

  }
}

void TestElements::testBoxes( ) {
  BoxManager manager;
  QDir examplesDir( QString( "%1/../examples/" ).arg( CURRENTDIR ) );
/*  qDebug( ) << "Current dir: " << CURRENTDIR; */
  QStringList entries;
  entries << "*.panda";
  QFileInfoList files = examplesDir.entryInfoList( entries );
  for( QFileInfo f : files ) {
    qDebug( ) << "FILE: " << f.absoluteFilePath( );
    Box box;
    manager.loadBox( &box, f.absoluteFilePath( ) );
  }
}
