// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <QDebug>
#include <QDir>

#include "common.h"
#include "filehelper.h"
#include "globalproperties.h"
#include "icnotfoundexception.h"

void FileHelper::verifyRecursion(const QString &fname)
{
    Q_UNUSED(fname);
    // TODO: BoxFileHelper::verifyRecursion
    //  std::string msg = "Oh no! I'm my own parent.\nSomething is not ok...";
    //  if( !parentFile.isEmpty( ) && ( fname == parentFile ) ) {
    //    throw( std::runtime_error( msg ) );
    //  }
    //  for( Box *box = parentBox; box != nullptr; box = box->getParentBox( ) ) {
    ////    qDebug( ) << "File: " << box->getFile( );
    //    if( box->getFile( ) == fname ) {
    //      throw( std::runtime_error( msg ) );
    //    }
    //  }
}
