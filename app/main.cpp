#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setOrganizationName("WPanda");
  a.setApplicationName("WiredPanda");
  a.setApplicationVersion("1.7");
  MainWindow w;
  w.show();
  if(argc == 2 ) {
    w.open(QString(argv[1]));
  }
  return a.exec();
}

