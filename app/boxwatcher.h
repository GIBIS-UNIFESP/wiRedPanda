#ifndef BOXWATCHER_H
#define BOXWATCHER_H

#include <QFileSystemWatcher>
#include <QString>

// TODO: Marked for removal.
class BoxWatcher {
  QString m_filename;
  QFileSystemWatcher m_fileWatcher;

public:
  BoxWatcher( const QString &filename );
  void fileUpdated( const QString & );

};

#endif // BOXWATCHER_H
