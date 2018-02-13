#include "box.h"
#include "boxfilehelper.h"
#include "boxprototype.h"

#include <QFileInfo>

BoxPrototype::BoxPrototype( const QString &fileName ) : m_fileName( fileName ) {

}

QString BoxPrototype::fileName( ) const {
  return( m_fileName );
}

QString BoxPrototype::baseName( ) const {
  return( QFileInfo( m_fileName ).baseName( ) );
}

void BoxPrototype::insertBoxObserver( Box *box ) {
  if( !boxObservers.contains( box ) ) {
    boxObservers.append( box );
  }
}

void BoxPrototype::removeBoxObserver( Box *box ) {
  if( boxObservers.contains( box ) ) {
    boxObservers.removeAll( box );
  }
}

int BoxPrototype::inputSize( ) {
  return( boxImpl.getInputSize( ) );
}

int BoxPrototype::outputSize( ) {
  return( boxImpl.getOutputSize( ) );
}

QString BoxPrototype::inputLabel( int index ) {
  return( boxImpl.inputLabels[ index ] );
}

QString BoxPrototype::outputLabel( int index ) {
  return( boxImpl.outputLabels[ index ] );
}

bool BoxPrototype::defaultInputValue( int index ) {
  return( boxImpl.defaultInputValues[ index ] );
}

bool BoxPrototype::isInputRequired( int index ) {
  return( boxImpl.requiredInputs[ index ] );
}

void BoxPrototype::clear( ) {
  boxImpl.clear( );
}

void BoxPrototype::reload( ) {
  //TODO: Verify file recursion
//  verifyRecursion( fname );

  clear( );

  boxImpl.loadFile( m_fileName );
  for( Box *box : boxObservers ) {
    box->loadFile( m_fileName );
  }
}
