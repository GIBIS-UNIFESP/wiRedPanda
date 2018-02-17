#ifndef BOXMANAGER_H
#define BOXMANAGER_H

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>

class MainWindow;
class BoxPrototype;

class BoxManager : public QObject {
  Q_OBJECT
  QMap< QString, BoxPrototype* > m_boxes;
  MainWindow *m_mainWindow;

  QFileSystemWatcher m_fileWatcher;
public:
  BoxManager( MainWindow *mainWindow = 0, QObject *parent = 0 );
  virtual ~BoxManager( );

  void loadFile(QString fname , QString parentFile);
  void clear( );

  BoxPrototype* getPrototype( QString fname );

  static BoxManager* instance( );

private slots:
  void reloadFile( QString bname );

private:
  bool warnAboutFileChange( const QString &fileName );

  static BoxManager *globalBoxManager;
};

#endif // BOXMANAGER_H
