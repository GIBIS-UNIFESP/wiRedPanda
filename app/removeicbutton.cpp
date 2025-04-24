#include "removeicbutton.h"


RemoveIcButton::RemoveIcButton(QWidget *parent)
    : QPushButton(QIcon(":/toolbar/trashcan.svg"), "", parent)
{
    setToolTip("Delete IC from global list");
}
