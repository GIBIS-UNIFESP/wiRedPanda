#include "elementfactory.h"
#include "graphicelement.h"
#include "label.h"
#include "listitemwidget.h"

#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QVariant>

Label* ListItemWidget::getLabel( ) const {
  return( label );
}

void ListItemWidget::mousePressEvent( QMouseEvent* ) {
  label->startDrag( );
}

ListItemWidget::ListItemWidget( const QPixmap &pixmap, ElementType type, QString boxFileName,
                                QWidget *parent ) : QFrame( parent ) {
  QHBoxLayout *itemLayout = new QHBoxLayout( );
  QString name = ElementFactory::translatedName( type );
  if( type == ElementType::BOX ) {
    name = QFileInfo( boxFileName ).baseName( ).toUpper( );
  }
  itemLayout->setSpacing( 6 );
  itemLayout->setObjectName( QStringLiteral( "itemLayout" ) );
/*  itemLayout->setSizeConstraint( QLayout::SetFixedSize ); */

  setLayout( itemLayout );

  label = new Label( parent );
  label->setPixmapData( pixmap );
  label->setName( name );
  label->setAuxData( boxFileName );
  label->setElementType( type );


  nameLabel = new QLabel( name, this );
  nameLabel->setText( name );
  itemLayout->addWidget( label );
  itemLayout->addStretch( );
  itemLayout->addWidget( nameLabel );
  itemLayout->addStretch( );
  itemLayout->setMargin( 0 );
}

void ListItemWidget::updateName( ) {
  ElementType type = label->elementType( );
  if( type != ElementType::BOX ) {
    QString name = ElementFactory::translatedName( type );
    nameLabel->setText( name );
    label->setName( name );

  }
}
