// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "locateicbutton.h"

LocateIcButton::LocateIcButton(QWidget *parent)
    : QPushButton(QIcon(":/toolbar/folder.svg"), "", parent)
{
    setToolTip("Search for IC");
}

