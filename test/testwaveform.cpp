#include "simplewaveform.h"
#include "testwaveform.h"

void TestWaveForm::init( ) {
  editor = new Editor( this );
}

void TestWaveForm::cleanup( ) {
  delete editor;
}

void TestWaveForm::testDisplay4Bits( ) {
  QDir examplesDir( QString( "%1/../examples/" ).arg( CURRENTDIR ) );
  QString fileName = examplesDir.absoluteFilePath( "display-4bits.panda" );
  QFile pandaFile( fileName );
  QVERIFY( pandaFile.open( QFile::ReadOnly ) );
  QDataStream ds( &pandaFile );
  try {
    editor->load( ds );
  }
  catch( std::runtime_error &e ) {
    QFAIL( QString( "Could not load the file! Error: %1" ).arg( QString::fromStdString( e.what( ) ) ).toUtf8( ) );
  }

  QString generatedFile = QDir::temp( ).absoluteFilePath( "display.txt" );
  QFile outFile( generatedFile );
  QVERIFY( outFile.open( QFile::WriteOnly ) );
  QTextStream outStream( &outFile );
  QVERIFY( SimpleWaveform::saveToTxt( outStream, editor ) );

  outFile.flush( );
  outFile.close( );

  QFile firstFile( generatedFile );
  QFile secndFile( examplesDir.absoluteFilePath( "display-4bits.txt" ) );

  QVERIFY( firstFile.open( QFile::ReadOnly ) );
  QVERIFY( secndFile.open( QFile::ReadOnly ) );

  QCOMPARE( firstFile.readAll( ), secndFile.readAll( ) );

  firstFile.close( );
  secndFile.close( );
}
