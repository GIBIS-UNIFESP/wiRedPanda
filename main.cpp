#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;

  w.show();
  if(argc == 2 ) {
    w.open(QString(argv[1]));
  }
  return a.exec();
}
