// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <QDebug>
#include <QDir>

#include "common.h"
#include "filehelper.h"
#include "globalproperties.h"
#include "icnotfoundexception.h"

QFileInfo FileHelper::findICFile(const QString &fname, const QString &parentFile)
{
    COMMENT("Loading file: " << fname.toStdString() << ", parentFile: " << parentFile.toStdString(), 3);
    QFileInfo currentFile(GlobalProperties::currentFile);
    QFileInfo fileInfo(fname);
    QString myFile = fileInfo.fileName();

    auto setFileInfo = [&fileInfo](const QDir &dir, const QString &file) {
        fileInfo.setFile(dir, file);
        return fileInfo.isFile();
    };

//    auto fileName = [&fileInfo](const QDir &dir, const QString &file) {
//        fileInfo.setFile(dir, file);
//        return fileInfo.absoluteFilePath().toStdString();
//    };

    QDir subdir(currentFile.absolutePath());
    subdir.cdUp();

//    COMMENT("Possible files:\n" <<
//            fileName(QDir::current(), fileInfo.fileName()) << "\n" <<
//            fileName(QFileInfo(parentFile).absoluteDir(), myFile) << "\n" <<
//            fileName(currentFile.absoluteDir(), myFile) << "\n" <<
//            fileName(QDir(currentFile.absolutePath() + "/boxes"), myFile) << "\n" <<
//            fileName(QDir(subdir.absolutePath() + "/boxes"), myFile), 0);

    if (!fileInfo.isFile() &&
        !setFileInfo(QDir::current(), fileInfo.fileName()) &&
        !setFileInfo(QFileInfo(parentFile).absoluteDir(), myFile) &&
        !setFileInfo(currentFile.absoluteDir(), myFile) &&
        !setFileInfo(QDir(currentFile.absolutePath() + "/boxes"), myFile) &&
        !setFileInfo(QDir(subdir.absolutePath() + "/boxes"), myFile)) {
        std::cerr << "Error: This file does not exist: " << fname.toStdString() << std::endl;
        throw(ICNotFoundException(QString(tr("IC linked file \"%1\" could not be found!\n"
                                             "Do you want to find this file?")).arg(fname).toStdString(),nullptr));
    }
    return fileInfo;
}

QFileInfo FileHelper::findSkinFile(const QString &fname)
{
    qDebug() << "Loading file: " << fname;
    QFileInfo currentFile(GlobalProperties::currentFile);
    QFileInfo fileInfo(fname);
    QString myFile = fileInfo.fileName();

    auto setFileInfo = [&fileInfo](const QDir &dir, const QString &file) {
        fileInfo.setFile(dir, file);
        return fileInfo.isFile();
    };

    if (!fileInfo.isFile() &&
        !setFileInfo(QDir::current(), fileInfo.fileName()) &&
        !setFileInfo(currentFile.absoluteDir(), myFile) &&
        !setFileInfo(QDir(currentFile.absolutePath() + "/skins/"), myFile)) {
        std::cerr << "Error: This file does not exist: " << fname.toStdString() << std::endl;
    }

    COMMENT("FileInfo found: " << fileInfo.absoluteFilePath().toStdString(), 0);
    return fileInfo;
}

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
