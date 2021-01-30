#include <QStringList>

#include "common.h"
#include "globalproperties.h"

double GlobalProperties::toDouble(QString txtVersion, bool *ok)
{
    if (txtVersion.contains("-")) {
        txtVersion = txtVersion.split("-").first();
    }

    if (txtVersion.contains(".")) {
        txtVersion = txtVersion.split(".")[0] + "." + txtVersion.split(".")[1];
    }

    return txtVersion.toDouble(ok);
}

double loadVersion()
{
    QString txtVersion(APP_VERSION);
    bool ok;
    double version = GlobalProperties::toDouble(txtVersion, &ok);
    if (!ok || (qFuzzyIsNull(version))) {
        throw std::runtime_error(ERRORMSG("INVALID VERSION NUMBER!"));
    }
    return (version);
}

QString GlobalProperties::currentFile = QString();
double GlobalProperties::version = loadVersion();

bool GlobalProperties::soundEnabled = true;
