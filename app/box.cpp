#include "box.h"
#include "editor.h"
#include "globalproperties.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPointF>
#include <QProcess>
#include <iostream>
#include <nodes/qneconnection.h>

Box::Box(Editor * editor, QGraphicsItem *parent ) : GraphicElement( 0, 0, 0, 0, parent ), simulationController(
    &myScene ) {
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
  setObjectName( "BOX" );
  connect( &watcher, &QFileSystemWatcher::fileChanged, this, &Box::fileChanged );
  isAskingToReload = false;
}

Box::~Box( ) {
}


ElementType Box::elementType( ) {
  return( ElementType::BOX );
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
  QMutexLocker locker( &mutex );
  for( int inputPort = 0; inputPort < inputMap.size( ); ++inputPort ) {
    inputMap.at( inputPort )->setValue( inputs( ).at( inputPort )->value( ) );
  }
  simulationController.update( );
  for( int outputPort = 0; outputPort < outputMap.size( ); ++outputPort ) {
    outputs( ).at( outputPort )->setValue( outputMap.at( outputPort )->value( ) );
  }
}

void Box::loadFile( QString fname ) {
  setToolTip( fname );
  QMutexLocker locker( &mutex );
  watcher.addPath( fname );
  QFileInfo fileInfo( fname );
  QString myFile = fileInfo.fileName( );
/*  qDebug( ) << "Trying to load (1): " << fileInfo.absoluteFilePath( ); */
  if( !fileInfo.exists( ) ) {
    fileInfo.setFile( QDir::current( ), fileInfo.fileName( ) );
/*    qDebug( ) << "Trying to load (2): " << fileInfo.absoluteFilePath( ); */
    if( !fileInfo.exists( ) ) {
      fileInfo.setFile( QFileInfo( parentFile ).absoluteDir( ), myFile );
/*
 *      qDebug() << "Parent file: " << parentFile;
 *      qDebug( ) << "Trying to load (3): " << fileInfo.absoluteFilePath( );
 */
      if( !fileInfo.exists( ) ) {
        QFileInfo currentFile( GlobalProperties::currentFile );
        fileInfo.setFile( currentFile.absoluteDir( ), myFile );
        if( !fileInfo.exists( ) ) {
          std::cerr << "Error: This file does not exists: " << fname.toStdString( ) << std::endl;
          throw( std::runtime_error( QString(
                                       "Box linked file \"%1\" could not be found!\n"
                                       "Do you want to find this file?" )
                                     .arg( fname ).toStdString( ) ) );
          return;
        }
      }
    }
  }
  fname = fileInfo.absoluteFilePath( );
  qDebug( ) << "Successfully opened box file \"" << fileInfo.absoluteFilePath( ) << "\"!";
  if( getLabel( ).isEmpty( ) ) {
    setLabel( fileInfo.baseName( ).toUpper( ) );
  }
  m_file = fileInfo.absoluteFilePath( );

  inputMap.clear( );
  outputMap.clear( );
  myScene.clear( );

  QFile file( fileInfo.absoluteFilePath( ) );
  if( file.open( QFile::ReadOnly ) ) {

    QDataStream ds( &file );

    QString str;
    ds >> str;
    if( !str.startsWith( QApplication::applicationName( ) ) ) {
      throw( std::runtime_error( "Invalid file format. (BOX)" ) );
    }
    double version = str.split( " " ).at( 1 ).toDouble( );
    QRectF rect;
    if( version >= 1.4 ) {
      ds >> rect;
    }
    QMap< quint64, QNEPort* > portMap;
    while( !ds.atEnd( ) ) {
      int type;
      ds >> type;
      if( type == GraphicElement::Type ) {
        quint64 elmType;
        ds >> elmType;
        GraphicElement *elm = editor->getFactory().buildElement( ( ElementType ) elmType, editor );
        if( elm ) {
          elm->load( ds, portMap, version );
          if( elm->elementType( ) == ElementType::BOX ) {
            Box *childBox = ( Box* ) elm;
            childBox->setParentFile( fname );
            if(!editor->loadBox(childBox, childBox->getFile())){
              throw( std::runtime_error( "Failed to load box element." ) );
            }
            break;
          }
          myScene.addItem( elm );
          switch( elm->elementType( ) ) {
              case ElementType::BUTTON:
              case ElementType::SWITCH:
              case ElementType::CLOCK: {
              foreach( QNEPort * port, elm->outputs( ) ) {
                inputMap.append( port );
              }
              elm->disable( );
              break;
            }
              case ElementType::DISPLAY:
              case ElementType::LED: {
              foreach( QNEPort * port, elm->inputs( ) ) {
                outputMap.append( port );
              }
              break;
                default:
                break;
            }
          }
        }
        else {
          throw( std::runtime_error( "Could not build element." ) );
        }
      }
      else if( type == QNEConnection::Type ) {
        QNEConnection *conn = new QNEConnection( 0 );
        conn->load( ds, portMap );
        myScene.addItem( conn );
      }
      else {
        throw( std::runtime_error( "Box opened with errors." ) );
        return;
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
      QString lb = inputMap.at( port )->graphicElement( )->getLabel( );
      if( lb.isEmpty( ) ) {
        lb = inputMap.at( port )->graphicElement( )->objectName( );
      }
      inputs( ).at( port )->setName( lb );
    }
    for( int port = 0; port < outputSize( ); ++port ) {
      QString lb = outputMap.at( port )->graphicElement( )->getLabel( );
      if( lb.isEmpty( ) ) {
        lb = outputMap.at( port )->graphicElement( )->objectName( );
      }
      outputs( ).at( port )->setName( lb );
    }
  }
}

QString Box::getParentFile( ) const {
  return( parentFile );
}

void Box::setParentFile( const QString &value ) {
  parentFile = value;
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
        p1 = map[ j + 1 ]->pos( );
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
