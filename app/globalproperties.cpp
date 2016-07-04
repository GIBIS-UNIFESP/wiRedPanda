#include "globalproperties.h"
#include <QStringList>

QString GlobalProperties::currentFile = QString();
double GlobalProperties::version = QString(APP_VERSION).split("-").first().toDouble();
