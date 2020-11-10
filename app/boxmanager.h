#ifndef BOXMANAGER_H
#define BOXMANAGER_H

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>

class MainWindow;
class BoxPrototype;
class Box;

class BoxManager : public QObject {
  Q_OBJECT
  QMap< QString, BoxPrototype* > boxes;
  MainWindow *mainWindow;

  QFileSystemWatcher fileWatcher;
public:
  BoxManager( MainWindow *mainWindow = nullptr, QObject *parent = nullptr );
  virtual ~BoxManager( );
  void clear( );
  bool loadBox( Box *box, QString fname, QString parentFile = "" );
  BoxPrototype* getPrototype( QString fname );
  static BoxManager* instance( );

  bool updatePrototypeFilePathName( QString sourceName, QString targetName );
signals:
  void updatedBox( );

private slots:
  void reloadFile( QString bname );

private:
  bool tryLoadFile( QString &fname, QString parentFile );
  void loadFile( QString &fname, QString parentFile );
  bool warnAboutFileChange( const QString &fileName );

  static BoxManager *globalBoxManager;
  void updateRecentBoxes( const QString &fname );
};

#endif // BOXMANAGER_H
