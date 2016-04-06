#include "box.h"
#include "boxnotfoundexception.h"
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
  QPixmap pixmap( ":/basic/box.png" );
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
  setPixmap( pixmap );
  setOutputsOnTop( true );
  setPortName( "BOX" );
  connect( &watcher, &QFileSystemWatcher::fileChanged, this, &Box::fileChanged );
  isAskingToReload = false;
  parentBox = nullptr;
}

Box::~Box( ) {
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

#include <iostream>
void Box::updateLogic( ) {
  for( int inputPort = 0; inputPort < inputMap.size( ); ++inputPort ) {
    inputMap.at( inputPort )->setValue( input( inputPort )->value( ) );
  }
  QVector< GraphicElement* > elms( myScene.getElements( ) );
  elms = SimulationController::sortElements( elms );
  for( GraphicElement *elm : elms ) {
    if( !elm->disabled( ) ) {
      elm->updateLogic( );
    }
/*
 *    std::cout << elm->objectName().toStdString() <<  " : ";
 *    for( QNEPort * in : elm->inputs()) {
 *      std::cout << ( int ) in->value( ) << " ";
 *    }
 *    std::cout << " => ";
 *    for( QNEPort * out : elm->outputs()) {
 *      std::cout << ( int ) out->value( ) << " ";
 *    }
 *    std::cout <<  std::endl;
 */
  }
  for( int outputPort = 0; outputPort < outputMap.size( ); ++outputPort ) {
    output( outputPort )->setValue( outputMap.at( outputPort )->value( ) );
  }
}

void Box::verifyRecursion( QString fname ) {
  std::string msg = "Oh no! I'm my own parent.\nSomething is not ok...";
  if( !parentFile.isEmpty( ) && ( fname == parentFile ) ) {
    throw( std::runtime_error( msg ) );
  }
  for( Box *box = parentBox; box != nullptr; box = box->getParentBox( ) ) {
    qDebug( ) << "File: " << box->getFile( );
    if( box->getFile( ) == fname ) {
      throw( std::runtime_error( msg ) );
    }
  }
}


void Box::loadFile( QString fname ) {
  qDebug() << "Loading file:" << fname;
  QFileInfo fileInfo = findFile( fname );
  fname = fileInfo.absoluteFilePath( );
  setToolTip( fname );
  if( getLabel( ).isEmpty( ) ) {
    setLabel( fileInfo.baseName( ).toUpper( ) );
  }
  verifyRecursion( fname );
  m_file = fname;
  watcher.addPath( fname );


  QFile file( fname );
  if( file.open( QFile::ReadOnly ) ) {
    inputMap.clear( );
    outputMap.clear( );
    myScene.clear( );
    QDataStream ds( &file );
    QList< QGraphicsItem* > items = SerializationFunctions::load( editor, ds, fname, &myScene );
    for( QGraphicsItem *item : items ) {
      if( item->type( ) == GraphicElement::Type ) {
        GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
        if( elm ) {
          switch( elm->elementType( ) ) {
              case ElementType::BUTTON:
              case ElementType::SWITCH:
              case ElementType::CLOCK: {
              for( QNEPort *port : elm->outputs( ) ) {
                inputMap.append( port );
              }
              elm->disable( );
              break;
            }
              case ElementType::DISPLAY:
              case ElementType::LED: {
              for( QNEPort *port : elm->inputs( ) ) {
                outputMap.append( port );
              }
              break;
            }
              default: {
              break;
            }
          }
        }
      }
    }
  }
  setMinInputSz( inputMap.size( ) );
  setMaxInputSz( inputMap.size( ) );
  setMinOutputSz( outputMap.size( ) );
  setMaxOutputSz( outputMap.size( ) );
  setInputSize( inputMap.size( ) );
  setOutputSize( outputMap.size( ) );
  sortMap( inputMap );
  sortMap( outputMap );
  for( int port = 0; port < inputSize( ); ++port ) {
    GraphicElement *elm = inputMap.at( port )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !inputMap.at( port )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += inputMap.at( port )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    input( port )->setName( lb );
    if( elm->elementType( ) != ElementType::CLOCK ) {
      input( port )->setRequired( false );
      input( port )->setDefaultValue( inputMap.at( port )->value( ) );
      input( port )->setValue( inputMap.at( port )->value( ) );
    }
  }
  for( int port = 0; port < outputSize( ); ++port ) {
    GraphicElement *elm = outputMap.at( port )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !outputMap.at( port )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += outputMap.at( port )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    output( port )->setName( lb );
    output( port )->setValue( outputMap.at( port )->value( ) );
  }
}

QString Box::getParentFile( ) const {
  return( parentFile );
}

void Box::setParentFile( const QString &value ) {
  parentFile = value;
}

QFileInfo Box::findFile( QString fname ) {
  QFileInfo fileInfo( fname);
  QString myFile = fileInfo.fileName( );
  qDebug( ) << "Trying to load (1): " << fileInfo.absoluteFilePath( );
  if( !fileInfo.exists( ) ) {
    fileInfo.setFile( QDir::current( ), fileInfo.fileName( ) );
    qDebug( ) << "Trying to load (2): " << fileInfo.absoluteFilePath( );
    if( !fileInfo.exists( ) ) {
      fileInfo.setFile( QFileInfo( parentFile ).absoluteDir( ), myFile );

      qDebug( ) << "Parent file: " << parentFile;
      qDebug( ) << "Trying to load (3): " << fileInfo.absoluteFilePath( );
      if( !fileInfo.exists( ) ) {
        QFileInfo currentFile( GlobalProperties::currentFile );
        qDebug( ) << "Current file: " << currentFile.absoluteFilePath();
        qDebug( ) << "Trying to load (4): " << fileInfo.absoluteFilePath( );
        fileInfo.setFile( currentFile.absoluteDir( ), myFile );
        if( !fileInfo.exists( ) ) {

          std::cerr << "Error: This file does not exists: " << fname.toStdString( ) << std::endl;
          throw( BoxNotFoundException( QString(
                                         "Box linked file \"%1\" could not be found!\n"
                                         "Do you want to find this file?" )
                                       .arg( fname ).toStdString( ), this ) );
        }
      }
    }
  }
  return( fileInfo );
}

QString Box::getFile( ) const {
  return( m_file );
}

void Box::sortMap( QVector< QNEPort* > &map ) {
  /* BubbleSort */
  for( int i = map.size( ) - 1; i >= 1; i-- ) {
    for( int j = 0; j < i; j++ ) {
      QPointF p1 = map[ j ]->graphicElement( )->pos( );
      QPointF p2 = map[ j + 1 ]->graphicElement( )->pos( );
      if( p1 != p2 ) {
        if( p1.y( ) > p2.y( ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
        else if( ( p1.y( ) == p2.y( ) ) && ( p1.x( ) > p2.x( ) ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
      }
      else {
        p1 = map[ j ]->pos( );
        p2 = map[ j + 1 ]->pos( );
        if( p1.x( ) > p2.x( ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
        else if( ( p1.x( ) == p2.x( ) ) && ( p1.y( ) > p2.y( ) ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
      }
    }
  }
}

Box* Box::getParentBox( ) const {
  return( parentBox );
}

void Box::setParentBox( Box *value ) {
  setParentFile( value->getFile( ) );
  parentBox = value;
}


void Box::fileChanged( QString file ) {
  if( isAskingToReload ) {
    return;
  }
  isAskingToReload = true;
  QMessageBox msgBox;
  /*    msgBox.setParent(  ); */
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  msgBox.setText( tr( "The file %1 changed, do you want to reload?" ).arg( file ) );
  msgBox.setWindowModality( Qt::ApplicationModal );
  msgBox.setDefaultButton( QMessageBox::Yes );
  if( msgBox.exec( ) == QMessageBox::Yes ) {
    loadFile( file );
  }
  isAskingToReload = false;
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
