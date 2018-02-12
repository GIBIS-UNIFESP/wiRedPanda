#include "boxfilehelper.h"
#include "boxprototype.h"

#include <QFileInfo>

BoxPrototype::BoxPrototype( const QString &fileName ) : m_fileName( fileName ) {

}

QString BoxPrototype::fileName( ) const {
  return( m_fileName );
}

int BoxPrototype::inputSize( ) const {
  return( boxImpl.inputSize( ) );
}

int BoxPrototype::outputSize( ) const {
  return( boxImpl.outputSize( ) );
}

void BoxPrototype::clear( ) {
  boxImpl.clear( );
}

void BoxPrototype::reload( ) {
  //TODO: Verify file recursion
//  verifyRecursion( fname );

  clear( );

  boxImpl.loadFile( m_fileName );
}
