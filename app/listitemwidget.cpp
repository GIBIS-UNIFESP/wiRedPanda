#include "label.h"
#include "listitemwidget.h"

#include <QFrame>
#include <QHBoxLayout>


Label * ListItemWidget::getLabel() const
{
  return label;
}
ListItemWidget::ListItemWidget(const QPixmap & pixmap, QString name, QString pixName,  QString auxData, QWidget *parent ) : QWidget( parent ) {
  QHBoxLayout *itemLayout = new QHBoxLayout( );
  itemLayout->setSpacing( 6 );
  itemLayout->setObjectName( QStringLiteral( "itemLayout" ) );
  itemLayout->setSizeConstraint( QLayout::SetFixedSize );

  setObjectName( QString( "_listItem_" ) + name );
  setLayout( itemLayout );

  label = new Label( this );
  label->setObjectName( pixName );
  label->setPixmap( pixmap );
  label->setAuxData( auxData );

  QLabel *nameLabel = new QLabel( name );
  nameLabel->setText(name);
  itemLayout->addWidget( label );
  itemLayout->addWidget( nameLabel );
}
