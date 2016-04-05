#ifndef RECENTFILESCONTROLLER_H
#define RECENTFILESCONTROLLER_H

#include <QObject>


class RecentFilesController : public QObject {
  Q_OBJECT
  QString attrName;
public:
  static const int MaxRecentFiles = 10;
  explicit RecentFilesController( QString attrName, QObject *parent = 0 );
  void addFile( QString fname );
  QStringList getFiles();

signals:
  void recentFilesUpdated( );
};

#endif /* RECENTFILESCONTROLLER_H */
