#include "box.h"
#include "boxnotfoundexception.h"
#include "boxprototype.h"
#include "editor.h"
#include "globalproperties.h"
#include "serializationfunctions.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPointF>
#include <QProcess>
#include <inputswitch.h>
#include <iostream>
#include <nodes/qneconnection.h>

Box::Box( Editor *editor, QGraphicsItem *parent ) : GraphicElement( 0, 0, 0, 0, parent ) {
  this->editor = editor;
  setHasLabel( true );
  /*
   *  QTransform transform;
   *  transform.translate(pixmap.size().width() / 2, pixmap.size().height() / 2);
   *  transform.rotate(-90);
   *  transform.translate(-pixmap.size().width() / 2, -pixmap.size().height() / 2);
   */

  /*
   *  setRotatable(false);
   *  setRotation(90);
   */

  /*  setPixmap(pixmap.transformed(transform)); */
  setPixmap( ":/basic/box.png" );
  setOutputsOnTop( true );
  setPortName( "BOX" );
}

Box::~Box( ) {
  BoxPrototype *prototype = BoxManager::instance( )->getPrototype( m_file );
  if( prototype ) {
    prototype->removeBoxObserver( this );
  }
}


void Box::save( QDataStream &ds ) {
  GraphicElement::save( ds );
  ds << m_file;
}

void Box::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
  if( version >= 1.2 ) {
    ds >> m_file;
  }
}


void Box::updateLogic( ) {
  //TODO
//  for( int inputPort = 0; inputPort < inputMap.size( ); ++inputPort ) {
//    inputMap.at( inputPort )->setValue( input( inputPort )->value( ) );
//  }
//  for( GraphicElement *elm : elements ) {
//    if( !elm->disabled( ) ) {
//      elm->updateLogic( );
//    }
//  }
//  for( int outputPort = 0; outputPort < outputMap.size( ); ++outputPort ) {
//    output( outputPort )->setValue( outputMap.at( outputPort )->value( ) );
//  }
}


void Box::loadInputs( BoxPrototype *prototype ) {
  setMaxInputSz( prototype->inputSize( ) );
  setMinInputSz( prototype->inputSize( ) );
  setInputSize( prototype->inputSize( ) );
  for( int inputIdx = 0; inputIdx < prototype->inputSize( ); ++inputIdx ) {
    QNEPort *in = input( inputIdx );
    in->setName( prototype->inputLabel( inputIdx ) );
    in->setDefaultValue( prototype->defaultInputValue( inputIdx ) );
    in->setRequired( prototype->isInputRequired( inputIdx ) );
  }
}

void Box::loadOutputs( BoxPrototype *prototype ) {
  setMaxOutputSz( prototype->outputSize( ) );
  setMinOutputSz( prototype->outputSize( ) );
  setOutputSize( prototype->outputSize( ) );
  for( int outputIdx = 0; outputIdx < prototype->outputSize( ); ++outputIdx ) {
    QNEPort *in = output( outputIdx );
    in->setName( prototype->outputLabel( outputIdx ) );
  }
}

void Box::loadFile( QString fname ) {
  m_file = fname;
  setToolTip( fname );
  BoxPrototype *prototype = getPrototype( );
  prototype->insertBoxObserver( this );
  if( getLabel( ).isEmpty( ) ) {
    setLabel( prototype->baseName( ).toUpper( ) );
  }
  // Loading inputs
  loadInputs( prototype );
  // Loading outputs
  loadOutputs( prototype );
  updatePorts( );
}

QString Box::getFile( ) const {
  return( m_file );
}

BoxPrototype* Box::getPrototype( ) {
  return( BoxManager::instance( )->getPrototype( m_file ) );
}


void Box::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {
    QMessageBox msgBox;
    /*    msgBox.setParent(  ); */
    msgBox.setLocale( QLocale::Portuguese );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setText( tr( "Do you want to load this file?<br>%1" ).arg( m_file ) );
    msgBox.setWindowModality( Qt::ApplicationModal );
    msgBox.setDefaultButton( QMessageBox::Yes );
    if( msgBox.exec( ) == QMessageBox::Yes ) {
      QProcess *wPanda = new QProcess( scene( ) );
      QStringList args;
      args << m_file;
      wPanda->start( QCoreApplication::applicationFilePath( ), args );
    }
  }
}
