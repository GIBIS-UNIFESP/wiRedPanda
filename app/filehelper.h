#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QFileInfo>
#include <QString>


class FileHelper {

public:
  static QFileInfo findBoxFile( QString fname, QString parentFile );
  static QFileInfo findSkinFile( QString fname );

  static void verifyRecursion( QString fname );

private:
  FileHelper( ) {
  }
};

#endif // BOXFILEHELPER_H
