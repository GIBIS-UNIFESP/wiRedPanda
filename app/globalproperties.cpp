// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QStringList>

#include "globalproperties.h"

double GlobalProperties::toDouble(const QString &txtVersion, bool *ok)
{
    QString value = txtVersion;
    if (value.contains("-")) {
        value = value.split("-").first();
    }

    if (value.contains(".")) {
        value = value.split(".")[0] + "." + value.split(".")[1];
    }

    return value.toDouble(ok);
}

double loadVersion()
{
    QString txtVersion(APP_VERSION);
    bool ok;
    double version = GlobalProperties::toDouble(txtVersion, &ok);
    if (!ok || (qFuzzyIsNull(version))) {
        throw std::runtime_error("INVALID VERSION NUMBER!");
    }
    return version;
}

QString GlobalProperties::currentFile = QString();
double GlobalProperties::version = loadVersion();

bool GlobalProperties::soundEnabled = true;
