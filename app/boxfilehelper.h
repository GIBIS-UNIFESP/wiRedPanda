#ifndef BOXFILEHELPER_H
#define BOXFILEHELPER_H

#include <QFileInfo>
#include <QString>


class BoxFileHelper {

public:
  static QFileInfo findFile(QString fname);

  static QString verifyRecursion(QString fname);

private:
  BoxFileHelper(){}
};

#endif // BOXFILEHELPER_H
