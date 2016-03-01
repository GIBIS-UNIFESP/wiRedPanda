#include "elementeditor.h"
#include "ui_elementeditor.h"

#include <QDebug>

ElementEditor::ElementEditor( QWidget *parent ) : QWidget( parent ), ui( new Ui::ElementEditor ) {
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
}

ElementEditor::~ElementEditor( ) {
  delete ui;
}

void ElementEditor::setScene( QGraphicsScene *s ) {
  scene = s;
  connect( s, &QGraphicsScene::selectionChanged, this, &ElementEditor::selectionChanged );
}

void ElementEditor::setCurrentElement( GraphicElement *elm ) {
  element = elm;
  if( elm != NULL ) {
    setVisible( true );
    setEnabled( false );
    bool hasSomething = false;
    /* Label */
    ui->lineEditElementLabel->setVisible( element->hasLabel( ) );
    ui->label_labels->setVisible( element->hasLabel( ) );
    ui->lineEditElementLabel->setText( element->getLabel( ) );
    hasSomething |= element->hasLabel( );
    /* Color */
    ui->label_color->setVisible( element->hasColors( ) );
    ui->comboBoxColor->setVisible( element->hasColors( ) );
    if( element->hasColors( ) ) {
      ui->comboBoxColor->setCurrentText( element->color( ) );
    }
    hasSomething |= element->hasColors( );
    /* Frequency */
    ui->doubleSpinBoxFrequency->setVisible( element->hasFrequency( ) );
    ui->label_frequency->setVisible( element->hasFrequency( ) );
    ui->doubleSpinBoxFrequency->setValue( element->frequency( ) );
    hasSomething |= element->hasFrequency( );
    /* Input size */
    QString inputSz = QString::number( element->inputSize( ) );
    ui->comboBoxInputSz->clear( );
    for( int port = element->minInputSz( ); port <= element->maxInputSz( ); ++port ) {
      ui->comboBoxInputSz->addItem( QString::number( port ), port );
    }
    ui->label_inputs->setVisible( ui->comboBoxInputSz->count( ) >= 2 );
    ui->comboBoxInputSz->setVisible( ui->comboBoxInputSz->count( ) >= 2 );
    ui->comboBoxInputSz->setCurrentText( inputSz );
    hasSomething |= ( ui->comboBoxInputSz->count( ) >= 2 );
    /* Trigger */
    ui->trigger->setVisible( element->hasTrigger( ) );
    ui->label_trigger->setVisible( element->hasTrigger( ) );

    ui->trigger->setCurrentIndex( 0 );
    if( element->hasTrigger( ) ) {
      ui->trigger->setCurrentText( element->getTrigger( ).toString( ) );
    }
    hasSomething |= ( element->hasTrigger( ) );
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
  GraphicElement *elm = NULL;
  for( QGraphicsItem *item : items ) {
    if( item->type( ) == GraphicElement::Type ) {
      if( elm != NULL ) {
        setCurrentElement( NULL );
        return;
      }
      else {
        elm = qgraphicsitem_cast< GraphicElement* >( item );
      }
    }
  }
  setCurrentElement( elm );
}

void ElementEditor::apply( ) {
  if( ( element == NULL ) || ( isEnabled( ) == false ) ) {
    return;
  }
  QByteArray itemData;
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  element->save( dataStream );
  if( element->minInputSz( ) != element->maxInputSz( ) ) {
    element->setInputSize( ui->comboBoxInputSz->currentData( ).toInt( ) );
  }
  if( element->hasColors( ) ) {
    element->setColor( ui->comboBoxColor->currentText( ) );
  }
  if( element->hasLabel( ) ) {
    element->setLabel( ui->lineEditElementLabel->text( ) );
  }
  if( element->hasFrequency( ) ) {
    element->setFrequency( ui->doubleSpinBoxFrequency->value( ) );
  }
  if( element->hasTrigger( ) ) {
    element->setTrigger( QKeySequence( ui->trigger->currentText() ) );
  }
  emit elementUpdated( element, itemData );
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

void ElementEditor::on_trigger_currentIndexChanged(const QString &){
  apply();
}
