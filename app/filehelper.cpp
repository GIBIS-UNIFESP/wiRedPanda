// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filehelper.h"

#include "common.h"
#include "globalproperties.h"
#include "icnotfoundexception.h"

#include <QDebug>
#include <QDir>
#include <iostream>

void FileHelper::verifyRecursion(const QString &fname)
{
    Q_UNUSED(fname);
    // TODO: BoxFileHelper::verifyRecursion
    // std::string msg = "Oh no! I'm my own parent.\nSomething is not ok...";
    // if (!parentFile.isEmpty() && (fname == parentFile)) {
    //     throw(std::runtime_error(msg));
    // }
    // for (auto *box = parentBox; box != nullptr; box = box->getParentBox()) {
    //     // qCDebug(zero) << "File:" << box->getFile();
    //     if (box->getFile() == fname) {
    //         throw(std::runtime_error(msg));
    //     }
    // }
}
