#include "box.h"
#include "boxfilehelper.h"
#include "boxmapping.h"
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

int BoxPrototype::inputSize( ) const {
  return( boxImpl.getInputSize( ) );
}

int BoxPrototype::outputSize( ) const {
  return( boxImpl.getOutputSize( ) );
}

QString BoxPrototype::inputLabel( int index ) const {
  return( boxImpl.inputLabels[ index ] );
}

QString BoxPrototype::outputLabel( int index ) const {
  return( boxImpl.outputLabels[ index ] );
}

bool BoxPrototype::defaultInputValue( int index ) {
  return( boxImpl.inputs[ index ]->value( ) );
}

bool BoxPrototype::isInputRequired( int index ) {
  return( boxImpl.inputs[ index ]->isRequired( ) );
}

BoxMapping* BoxPrototype::generateMapping( ) const {
  return( new BoxMapping( fileName( ), boxImpl.elements, boxImpl.inputs, boxImpl.outputs ) );
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
