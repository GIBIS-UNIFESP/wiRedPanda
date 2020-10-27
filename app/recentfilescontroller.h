#ifndef RECENTFILESCONTROLLER_H
#define RECENTFILESCONTROLLER_H

#include <QObject>


class RecentFilesController : public QObject {
  Q_OBJECT
  QString attrName;
public:
  static const int MaxRecentFiles = 10;
  explicit RecentFilesController(QString _attrName, QObject *parent = nullptr ) : QObject( parent ), attrName(_attrName) {};
  void addFile( QString fname );
  QStringList getFiles( );

signals:
  void recentFilesUpdated( );
};


#endif /* RECENTFILESCONTROLLER_H */
