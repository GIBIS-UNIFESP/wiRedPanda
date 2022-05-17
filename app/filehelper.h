/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QFileInfo>
#include <QString>

class FileHelper : public QObject
{
    Q_OBJECT

public:
    FileHelper() = delete;

    static void verifyRecursion(const QString &fileName);
};

