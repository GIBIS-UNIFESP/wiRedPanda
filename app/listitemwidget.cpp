#include "label.h"
#include "listitemwidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVariant>


Label* ListItemWidget::getLabel( ) const {
  return( label );
}

void ListItemWidget::mousePressEvent( QMouseEvent * ) {
  label->startDrag();
}

ListItemWidget::ListItemWidget( const QPixmap &pixmap, QString name, QString pixName, QString auxData,
                                QWidget *parent ) : QWidget( parent ) {
  QHBoxLayout *itemLayout = new QHBoxLayout( );
  itemLayout->setSpacing( 6 );
  itemLayout->setObjectName( QStringLiteral( "itemLayout" ) );
  itemLayout->setSizeConstraint( QLayout::SetFixedSize );

  setObjectName( QString( "_listItem_" ) + name );
  setLayout( itemLayout );

  label = new Label( parent );
  label->setObjectName( pixName );
  label->setPixmap( pixmap );
  label->setAuxData( auxData );
  label->setProperty( "Name", name );

  QLabel *nameLabel = new QLabel( name, this );
  nameLabel->setText( name );
  itemLayout->addWidget( label );
  itemLayout->addWidget( nameLabel );
  itemLayout->setMargin(0);

}
