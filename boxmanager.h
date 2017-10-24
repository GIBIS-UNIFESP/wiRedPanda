#ifndef BOXMANAGER_H
#define BOXMANAGER_H

#include <QMap>
#include <QObject>


class LogicBox;
class MainWindow;

class BoxManager : public QObject {
  Q_OBJECT
  QMap< QString, LogicBox* > boxes;
  MainWindow *parent;
public:
  BoxManager( MainWindow *parent );

  void loadFile( QString fname );
  void clear( );

public slots:
  void updateFile( QString file );

};

#endif // BOXMANAGER_H
