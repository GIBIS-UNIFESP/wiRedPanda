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

ui->lineEditTrigger->setValidator(new QRegExpValidator(QRegExp("[a-z]| |[A-Z]|[0-9]"), this));
}

ElementEditor::~ElementEditor( ) {
  delete ui;
}

void ElementEditor::setScene( QGraphicsScene *s ) {
  scene = s;
  connect( s, &QGraphicsScene::selectionChanged, this, &ElementEditor::selectionChanged );
}

void addElementAction( QMenu *menu, GraphicElement *firstElm, ElementType type, bool hasSameType ) {
  if( !hasSameType || ( hasSameType && ( firstElm->elementType( ) != type ) ) ) {
    menu->addAction( QIcon( ElementFactory::getPixmap( type ) ), ElementFactory::typeToText( type ) );
  }
}

void ElementEditor::contextMenu( QPoint screenPos, Editor *editor ) {
  QMenu menu;
  QString renameActionText( tr( "Rename" ) );
  QString rotateActionText( tr( "Rotate" ) );
  QString freqActionText( tr( "Change frequency" ) );
  QString colorMenuText( tr( "Change color to..." ) );
  QString triggerActionText( tr( "Change trigger" ) );
  QString deleteActionText( tr( "Delete" ) );
  QString morphMenuText( tr( "Morph to..." ) );
  if( hasLabel ) {
    menu.addAction( QIcon( QPixmap( ":/toolbar/rename.png" ) ), renameActionText );
  }
  if( hasTrigger ) {
    menu.addAction( QIcon( ElementFactory::getPixmap(ElementType::BUTTON) ), triggerActionText );
  }
  if( hasRotation ) {
    menu.addAction( QIcon( QPixmap( ":/toolbar/rotateR.png" ) ), rotateActionText );
  }
  if( hasFrequency ) {
    menu.addAction( QIcon( ElementFactory::getPixmap(ElementType::CLOCK ) ), freqActionText );
  }
  QMenu *submenucolors = nullptr;
  if( hasColors ) {
    submenucolors = menu.addMenu( colorMenuText );
    for( int i = 0; i < ui->comboBoxColor->count( ); ++i ) {
      if( ui->comboBoxColor->currentIndex( ) != i ) {
        submenucolors->addAction( ui->comboBoxColor->itemIcon( i ), ui->comboBoxColor->itemText( i ) );
      }
    }
  }
  QMenu *submenumorph = nullptr;
  if( canMorph ) {
    submenumorph = menu.addMenu( morphMenuText );
    GraphicElement *firstElm = elements.front( );
    switch( firstElm->elementGroup( ) ) {
        case ElementGroup::GATE: {
        if( firstElm->inputSize( ) == 1 ) {
          addElementAction( submenumorph, firstElm, ElementType::NOT, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::NODE, hasSameType );
        }
        else {
          addElementAction( submenumorph, firstElm, ElementType::AND, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::OR, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::NAND, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::NOR, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::XOR, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::XNOR, hasSameType );
        }
        break;
      }
        case ElementGroup::INPUT: {
        addElementAction( submenumorph, firstElm, ElementType::BUTTON, hasSameType );
        addElementAction( submenumorph, firstElm, ElementType::SWITCH, hasSameType );
        addElementAction( submenumorph, firstElm, ElementType::CLOCK, hasSameType );
        addElementAction( submenumorph, firstElm, ElementType::VCC, hasSameType );
        addElementAction( submenumorph, firstElm, ElementType::GND, hasSameType );
        break;
      }
        case ElementGroup::MEMORY: {
        if( firstElm->inputSize( ) == 2 ) {
          addElementAction( submenumorph, firstElm, ElementType::TLATCH, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::DLATCH, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::JKLATCH, hasSameType );
        }
        else if( firstElm->inputSize( ) == 4 ) {
          addElementAction( submenumorph, firstElm, ElementType::DFLIPFLOP, hasSameType );
          addElementAction( submenumorph, firstElm, ElementType::TFLIPFLOP, hasSameType );
        }
        break;
      }
    }
    if( submenumorph->actions( ).size( ) == 0 ) {
      menu.removeAction( submenumorph->menuAction( ) );
    }
  }
  menu.addAction( QIcon( QPixmap( ":/toolbar/delete.png" ) ), deleteActionText );
  QAction *a = menu.exec( screenPos );
  if( a ) {
    if( a->text( ) == deleteActionText ) {
      emit sendCommand( new DeleteItemsCommand( scene->selectedItems( ) ) );
    }
    else if( a->text( ) == renameActionText ) {
      ui->lineEditElementLabel->setFocus( );
      ui->lineEditElementLabel->selectAll();
    }
    else if( a->text( ) == rotateActionText ) {
      emit sendCommand( new RotateCommand( elements.toList( ), 90.0 ) );
    }
    else if( a->text( ) == triggerActionText ) {
      ui->lineEditTrigger->setFocus( );
      ui->lineEditTrigger->selectAll();
    }
    else if( a->text( ) == freqActionText ) {
      ui->doubleSpinBoxFrequency->setFocus( );
    }
    else if( submenumorph && submenumorph->actions( ).contains( a ) ) {
      if( ElementFactory::textToType( a->text( ) ) != ElementType::UNKNOWN ) {
        sendCommand( new MorphCommand( elements, ElementFactory::textToType( a->text( ) ), editor ) );
      }
    }
    else if( submenucolors && submenucolors->actions( ).contains( a ) ) {
      ui->comboBoxColor->setCurrentText( a->text( ) );
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
    hasSameType = true;
    GraphicElement *firstElement = elements.front( );
    for( GraphicElement *elm : elements ) {
      hasLabel &= elm->hasLabel( );
      hasColors &= elm->hasColors( );
      hasFrequency &= elm->hasFrequency( );
      minimum = std::max( minimum, elm->minInputSz( ) );
      maximum = std::min( maximum, elm->maxInputSz( ) );
      hasTrigger &= elm->hasTrigger( );
      hasRotation &= elm->rotatable( );

      hasSameLabel &= elm->getLabel( ) == firstElement->getLabel( );
      hasSameColors &= elm->getColor( ) == firstElement->getColor( );
      hasSameFrequency &= elm->getFrequency( ) == firstElement->getFrequency( );
      hasSameInputSize &= elm->inputSize( ) == firstElement->inputSize( );
      hasSameTrigger &= elm->getTrigger( ) == firstElement->getTrigger( );
      hasSameType &= elm->elementType( ) == firstElement->elementType( );
      canMorph &= elm->elementGroup( ) == firstElement->elementGroup( );
      canMorph &= elm->inputSize( ) == firstElement->inputSize( );
      canMorph &= elm->outputSize( ) == firstElement->outputSize( );
    }
    canChangeInputSize = ( minimum < maximum );
    hasSomething |= hasLabel | hasColors | hasFrequency;
    hasSomething |= canChangeInputSize | hasTrigger;


    /* Labels */
    ui->lineEditElementLabel->setVisible( hasLabel );
    ui->label_labels->setVisible( hasLabel );
    if( hasLabel ) {
      if( hasSameLabel ) {
        ui->lineEditElementLabel->setText( firstElement->getLabel( ) );
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
        ui->comboBoxColor->setCurrentText( firstElement->getColor( ) );
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
        ui->doubleSpinBoxFrequency->setValue( firstElement->getFrequency( ) );
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
        QString inputSz = QString::number( firstElement->inputSize( ) );
        ui->comboBoxInputSz->removeItem( ui->comboBoxInputSz->findText( manyIS ) );
        ui->comboBoxInputSz->setCurrentText( inputSz );
      }
      else {
        ui->comboBoxInputSz->setCurrentText( manyIS );
      }
    }
    /* Trigger */
    ui->lineEditTrigger->setVisible( hasTrigger );
    ui->label_trigger->setVisible( hasTrigger );
    if( hasTrigger ) {
      if( hasSameTrigger ) {
        ui->lineEditTrigger->setText(firstElement->getTrigger().toString());
      }
      else {
        ui->lineEditTrigger->setText( manyTriggers );
      }
    }
    setEnabled( hasSomething );
    setVisible( hasSomething );
  }
  else {
    setVisible( false );
    ui->lineEditElementLabel->setText( "" );  }
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
    if( elm->hasColors( ) && ( ui->comboBoxColor->currentText( ) != manyColors ) ) {
      elm->setColor( ui->comboBoxColor->currentText( ) );
    }
    if( elm->hasLabel( ) && ( ui->lineEditElementLabel->text( ) != manyLabels ) ) {
      elm->setLabel( ui->lineEditElementLabel->text( ) );
    }
    if( elm->hasFrequency( ) && ( ui->doubleSpinBoxFrequency->text( ) != manyFreq ) ) {
      elm->setFrequency( ui->doubleSpinBoxFrequency->value( ) );
    }
    if( elm->hasTrigger( ) && ( ui->lineEditTrigger->text( ) != manyTriggers ) ) {
      elm->setTrigger( QKeySequence( ui->lineEditTrigger->text( ) ) );
    }
  }
  emit sendCommand( new UpdateCommand( elements, itemData ) );
}

void ElementEditor::on_lineEditElementLabel_editingFinished( ) {
  apply( );
}

void ElementEditor::on_comboBoxInputSz_currentIndexChanged( int index ) {
  Q_UNUSED( index );
  if( ( elements.isEmpty( ) ) || ( isEnabled( ) == false ) ) {
    return;
  }
  if( canChangeInputSize && ( ui->comboBoxInputSz->currentText( ) != manyIS ) ) {
    emit sendCommand( new ChangeInputSZCommand( elements, ui->comboBoxInputSz->currentData( ).toInt( ) ) );
  }
}

void ElementEditor::on_doubleSpinBoxFrequency_editingFinished( ) {
  apply( );
}

void ElementEditor::on_comboBoxColor_currentIndexChanged( int index ) {
  Q_UNUSED( index );
  apply( );
}

void ElementEditor::on_lineEditTrigger_textChanged( const QString &cmd ) {
  ui->lineEditTrigger->setText(cmd.toUpper());
}

void ElementEditor::on_lineEditTrigger_editingFinished( ) {
  apply();
}
