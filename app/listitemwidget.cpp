#include "label.h"
#include "listitemwidget.h"
#include "graphicelement.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QVariant>


Label* ListItemWidget::getLabel( ) const {
  return( label );
}

void ListItemWidget::mousePressEvent( QMouseEvent * ) {
  label->startDrag();
}

ListItemWidget::ListItemWidget( const QPixmap &pixmap, QString name, ElementType elementType, QString auxData,
                                QWidget *parent ) : QFrame( parent ) {
  QHBoxLayout *itemLayout = new QHBoxLayout( );
  itemLayout->setSpacing( 6 );
  itemLayout->setObjectName( QStringLiteral( "itemLayout" ) );
//  itemLayout->setSizeConstraint( QLayout::SetFixedSize );

  setObjectName( QString( "_listItem_" ) + name );
  setLayout( itemLayout );

  label = new Label( parent );
  label->setPixmap( pixmap );
  label->setName(name);
  label->setAuxData( auxData );
  label->setElementType(elementType);


  QLabel *nameLabel = new QLabel( name, this );
  nameLabel->setText( name );
  itemLayout->addWidget( label );
  itemLayout->addStretch();
  itemLayout->addWidget( nameLabel );
  itemLayout->addStretch();
  itemLayout->setMargin(0);
}
