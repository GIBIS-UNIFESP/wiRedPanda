#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "editor.h"

#include <QMainWindow>
#include <QGraphicsScene>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  Editor * editor;
  QGraphicsScene * scene;
};

#endif // MAINWINDOW_H
