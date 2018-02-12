#ifndef BOXMANAGER_H
#define BOXMANAGER_H

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>

class MainWindow;
class BoxPrototype;

class BoxManager : public QObject {
  Q_OBJECT
  QMap< QString, BoxPrototype* > boxes;
  MainWindow *mainWindow;

  QFileSystemWatcher fileWatcher;
public:
  BoxManager( MainWindow *mainWindow = 0, QObject *parent = 0 );
  virtual ~BoxManager( );

  void loadFile( QString fname );
  void clear( );

private slots:
  void reloadFile( QString fname );

private:
  bool warnAboutFileChange( const QString &fileName );
};

#endif // BOXMANAGER_H
