#include "logicbox.h"

#include <QDebug>



QString LogicBox::getFilename( ) const {
  return( filename );
}

void LogicBox::_updateLogic( const std::vector< bool > &inputs ) {
//  setOutputValue( );
}


LogicBox::LogicBox( QString fname ) : LogicElement( 0, 0 ) {
  qDebug( ) << "Loading " << fname;
}

LogicBox::~LogicBox( ) {

}

LogicBox* LogicBox::loadFile( QString fname ) {
  LogicBox *box = new LogicBox( fname );

  return( box );
}

void LogicBox::reload( ) {

}
