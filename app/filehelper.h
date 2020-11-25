#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QFileInfo>
#include <QString>


class FileHelper : public QObject {
  Q_OBJECT
public:
  static QFileInfo findICFile( QString fname, QString parentFile );
  static QFileInfo findSkinFile( QString fname );

  static void verifyRecursion( QString fname );

private:
  FileHelper( QObject *parent = nullptr ) : QObject( parent ) {
  }
};

#endif // FILEHELPER_H
