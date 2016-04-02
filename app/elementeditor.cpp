#include "commands.h"
#include "editor.h"
#include "elementeditor.h"
#include "ui_elementeditor.h"
#include <QDebug>
#include <QMenu>
#include <cmath>

ElementEditor::ElementEditor( QWidget *parent ) : QWidget( parent ), ui( new Ui::ElementEditor ) {
  manyLabels = tr( "<Many labels>" );
  manyColors = tr( "<Many colors>" );
  manyIS = tr( "<Many values>" );
  manyFreq = tr( "<Many values>" );
  manyTriggers = tr( "<Many triggers>" );

  ui->setupUi( this );
  setEnabled( false );
  setVisible( false );

  ui->trigger->addItem( QString( tr( "None" ) ) );
  for( int i = 0; i < 5; i++ ) {
    ui->trigger->addItem( QKeySequence( QString( "%1" ).arg( i ) ).toString( ) );
  }
  for( char i = 'A'; i < 'F'; i++ ) {
    ui->trigger->addItem( QKeySequence( QString( "%1" ).arg( i ) ).toString( ) );
  }
  ui->trigger->addItem( QKeySequence( 'W' ).toString( ) );
  ui->trigger->addItem( QKeySequence( "S" ).toString( ) );
  ui->trigger->addItem( QKeySequence( "D" ).toString( ) );
  ui->trigger->addItem( QKeySequence( 'Q' ).toString( ) );
  ui->trigger->addItem( QKeySequence( "R" ).toString( ) );
  ui->trigger->addItem( QKeySequence( "T" ).toString( ) );
  ui->trigger->addItem( QKeySequence( "Y" ).toString( ) );

}

ElementEditor::~ElementEditor( ) {
  delete ui;
}

void ElementEditor::setScene( QGraphicsScene *s ) {
  scene = s;
  connect( s, &QGraphicsScene::selectionChanged, this, &ElementEditor::selectionChanged );
}

void ElementEditor::contextMenu( QPoint screenPos, Editor * editor ) {
  QMenu menu;
  QString renameAction( tr( "Rename" ) );
  QString rotateAction( tr( "Rotate" ) );
  QString changeColorAction( tr( "Change Color" ) );
  QString deleteAction( tr( "Delete" ) );
  QString morphMenu( tr( "Morph to..." ) );
  if( hasLabel ) {
    menu.addAction( renameAction );
  }
  if( hasRotation ) {
    menu.addAction( rotateAction );
  }
  if( hasColors ) {
    menu.addAction( changeColorAction );
  }
  if( canMorph && ( elements.front( )->elementGroup( ) == ElementGroup::GATE ) ) {
    QMenu *submenu = menu.addMenu( morphMenu );
    if( elements.front( )->inputSize( ) == 1 ) {
      submenu->addAction( "NOT" );
      submenu->addAction( "NODE" );
    }
    else {
      submenu->addAction( "AND" );
      submenu->addAction( "OR" );
      submenu->addAction( "NAND" );
      submenu->addAction( "NOR" );
      submenu->addAction( "XOR" );
      submenu->addAction( "XNOR" );
    }
  }
  menu.addAction( deleteAction );
  QAction *a = menu.exec( screenPos );
  if( a ) {
    if( a->text( ) == deleteAction ) {
      emit sendCommand( new DeleteItemsCommand( scene->selectedItems( ) ) );
    }
    else if( a->text( ) == renameAction ) {
      ui->lineEditElementLabel->setFocus( );
    }
    else if( a->text( ) == rotateAction ) {
      emit sendCommand( new RotateCommand( elements.toList( ), 90.0 ) );
    }
    else if( a->text( ) == changeColorAction ) {
      if( !hasSameColors ) {
        ui->comboBoxColor->removeItem( ui->comboBoxColor->findText( manyColors ) );
      }
      int nxtIdx = ( ui->comboBoxColor->currentIndex( ) + 1 ) % ui->comboBoxColor->count( );
      ui->comboBoxColor->setCurrentIndex( nxtIdx );
    }else if ( ElementFactory::textToType(a->text()) != ElementType::UNKNOWN ){
      sendCommand( new MorphCommand( elements, ElementFactory::textToType(a->text()), editor ));
    }
  }
}


void ElementEditor::setCurrentElements( const QVector< GraphicElement* > &elms ) {
  elements = elms;
  if( !elms.isEmpty( ) ) {
    hasSomething = false;
    hasLabel = hasColors = hasFrequency = canChangeInputSize = hasTrigger = true;
    hasRotation = true;
    setVisible( true );
    setEnabled( false );
    int minimum = 0, maximum = 100000000;
    hasSameLabel = hasSameColors = hasSameFrequency = true;
    hasSameInputSize = hasSameTrigger = canMorph = true;
    for( GraphicElement *elm : elements ) {
      hasLabel &= elm->hasLabel( );
      hasColors &= elm->hasColors( );
      hasFrequency &= elm->hasFrequency( );
      minimum = std::max( minimum, elm->minInputSz( ) );
      maximum = std::min( maximum, elm->maxInputSz( ) );
      hasTrigger &= elm->hasTrigger( );
      hasRotation &= elm->rotatable( );

      hasSameLabel &= elm->getLabel( ) == elements.front( )->getLabel( );
      hasSameColors &= elm->color( ) == elements.front( )->color( );
      hasSameFrequency &= elm->frequency( ) == elements.front( )->frequency( );
      hasSameInputSize &= elm->inputSize( ) == elements.front( )->inputSize( );
      hasSameTrigger &= elm->getTrigger( ) == elements.front( )->getTrigger( );
      canMorph &= elm->elementGroup( ) == elements.front( )->elementGroup( );
      canMorph &= elm->inputSize( ) == elements.front( )->inputSize( );
      canMorph &= elm->outputSize( ) == elements.front( )->outputSize( );
    }
    canChangeInputSize = ( minimum < maximum );
    hasSomething |= hasLabel | hasColors | hasFrequency;
    hasSomething |= canChangeInputSize | hasTrigger;


    /* Labels */
    ui->lineEditElementLabel->setVisible( hasLabel );
    ui->label_labels->setVisible( hasLabel );
    if( hasLabel ) {
      if( hasSameLabel ) {
        ui->lineEditElementLabel->setText( elements.front( )->getLabel( ) );
      }
      else {
        ui->lineEditElementLabel->setText( manyLabels );
      }
    }
    /* Color */
    ui->label_color->setVisible( hasColors );
    ui->comboBoxColor->setVisible( hasColors );
    if( ui->comboBoxColor->findText( manyColors ) == -1 ) {
      ui->comboBoxColor->addItem( manyColors );
    }
    if( hasColors ) {
      if( hasSameColors ) {
        ui->comboBoxColor->removeItem( ui->comboBoxColor->findText( manyColors ) );
        ui->comboBoxColor->setCurrentText( elements.front( )->color( ) );
      }
      else {
        ui->comboBoxColor->setCurrentText( manyColors );
      }
    }
    /* Frequency */
    ui->doubleSpinBoxFrequency->setVisible( hasFrequency );
    ui->label_frequency->setVisible( hasFrequency );
    if( hasFrequency ) {
      if( hasSameFrequency ) {
        ui->doubleSpinBoxFrequency->setMinimum( 0.5 );
        ui->doubleSpinBoxFrequency->setSpecialValueText( QString( ) );
        ui->doubleSpinBoxFrequency->setValue( elements.front( )->frequency( ) );
      }
      else {
        ui->doubleSpinBoxFrequency->setMinimum( 0.0 );
        ui->doubleSpinBoxFrequency->setSpecialValueText( manyFreq );
        ui->doubleSpinBoxFrequency->setValue( 0.0 );
      }
    }
    /* Input size */
    ui->comboBoxInputSz->clear( );
    ui->label_inputs->setVisible( canChangeInputSize );
    ui->comboBoxInputSz->setVisible( canChangeInputSize );
    for( int port = minimum; port <= maximum; ++port ) {
      ui->comboBoxInputSz->addItem( QString::number( port ), port );
    }
    if( ui->comboBoxInputSz->findText( manyIS ) == -1 ) {
      ui->comboBoxInputSz->addItem( manyIS );
    }
    if( canChangeInputSize ) {
      if( hasSameInputSize ) {
        QString inputSz = QString::number( elements.front( )->inputSize( ) );
        ui->comboBoxInputSz->removeItem( ui->comboBoxInputSz->findText( manyIS ) );
        ui->comboBoxInputSz->setCurrentText( inputSz );
      }
      else {
        ui->comboBoxInputSz->setCurrentText( manyIS );
      }
    }
    /* Trigger */
    ui->trigger->setVisible( hasTrigger );
    ui->label_trigger->setVisible( hasTrigger );
    if( hasTrigger ) {
      if( ui->trigger->findText( manyTriggers ) == -1 ) {
        ui->trigger->addItem( manyTriggers );
      }
      if( hasSameTrigger ) {
        ui->trigger->removeItem( ui->trigger->findText( manyTriggers ) );
        QString tg = elements.front( )->getTrigger( ).toString( );
        if( tg.isEmpty( ) ) {
          ui->trigger->setCurrentText( "None" );
        }
        else {
          ui->trigger->setCurrentText( tg );
        }
      }
      else {
        ui->trigger->setCurrentText( manyTriggers );
      }
    }
    setEnabled( hasSomething );
    setVisible( hasSomething );
  }
  else {
    setVisible( false );
    ui->lineEditElementLabel->setText( "" );
  }
}

void ElementEditor::selectionChanged( ) {
  QList< QGraphicsItem* > items = scene->selectedItems( );
  QVector< GraphicElement* > elms;
  for( QGraphicsItem *item : items ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      if( elm != NULL ) {
        elms.append( elm );
      }
    }
  }
  setCurrentElements( elms );
}

void ElementEditor::apply( ) {
  if( ( elements.isEmpty( ) ) || ( isEnabled( ) == false ) ) {
    return;
  }
  QByteArray itemData;
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  for( GraphicElement *elm : elements ) {
    elm->save( dataStream );
    if( canChangeInputSize && ( ui->comboBoxInputSz->currentText( ) != manyIS ) ) {
      elm->setInputSize( ui->comboBoxInputSz->currentData( ).toInt( ) );
    }
    if( elm->hasColors( ) && ( ui->comboBoxColor->currentText( ) != manyColors ) ) {
      elm->setColor( ui->comboBoxColor->currentText( ) );
    }
    if( elm->hasLabel( ) && ( ui->lineEditElementLabel->text( ) != manyLabels ) ) {
      elm->setLabel( ui->lineEditElementLabel->text( ) );
    }
    if( elm->hasFrequency( ) && ( ui->doubleSpinBoxFrequency->text( ) != manyFreq ) ) {
      elm->setFrequency( ui->doubleSpinBoxFrequency->value( ) );
    }
    if( elm->hasTrigger( ) && ( ui->trigger->currentText( ) != manyTriggers ) ) {
      elm->setTrigger( QKeySequence( ui->trigger->currentText( ) ) );
    }
  }
  emit sendCommand( new UpdateCommand( elements, itemData ) );
}

void ElementEditor::on_lineEditElementLabel_editingFinished( ) {
  apply( );
}

void ElementEditor::on_comboBoxInputSz_currentIndexChanged( int index ) {
  Q_UNUSED( index );
  apply( );
}

void ElementEditor::on_doubleSpinBoxFrequency_editingFinished( ) {
  apply( );
}

void ElementEditor::on_comboBoxColor_currentIndexChanged( int index ) {
  Q_UNUSED( index );
  apply( );
}

void ElementEditor::on_trigger_currentIndexChanged( const QString & ) {
  apply( );
}
