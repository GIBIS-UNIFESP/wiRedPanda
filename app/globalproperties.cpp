#include "globalproperties.h"
#include <QStringList>
#include "common.h"
double loadVersion(){
  double version;
  QString txtVersion(APP_VERSION);
  if( txtVersion.contains("-") ){
    txtVersion = txtVersion.split("-").first();
  }
  bool ok;
  version = txtVersion.toDouble( &ok);
  if( !ok || version == 0. ){
    throw std::runtime_error(ERRORMSG("INVALID VERSION NUMBER!"));
  }
  return version;
}

QString GlobalProperties::currentFile = QString();
double GlobalProperties::version = loadVersion();
