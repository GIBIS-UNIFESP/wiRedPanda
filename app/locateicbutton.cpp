#include "locateicbutton.h"

LocateIcButton::LocateIcButton(QWidget *parent)
    : QPushButton(QIcon(":/toolbar/folder.svg"), "", parent)
{
    setToolTip("Search for IC");
}

