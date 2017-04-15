#ifndef GLOBALPROPERTIES_H
#define GLOBALPROPERTIES_H

#include <QString>
#define GLOBALCLK 10
class GlobalProperties {
public:

  static QString currentFile;
  static double version;

  static double toDouble(QString txtVersion, bool * ok);
};

#endif // GLOBALPROPERTIES_H
