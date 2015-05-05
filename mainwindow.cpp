#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow) {
  ui->setupUi(this);
  scene = new QGraphicsScene(this);
  ui->graphicsView->setScene(scene);
  ui->graphicsView->setBackgroundBrush(QBrush(QColor(Qt::gray)));
  ui->graphicsView->setRenderHint(QPainter::Antialiasing, true);
  ui->graphicsView->setAcceptDrops(true);
  editor = new  Editor(this);
  editor->install(scene);
  editor->setElementEditor(ui->widgetElementEditor);
  scene->setSceneRect(ui->graphicsView->rect());
//  ui->tabWidget->setTabEnabled(2,false);
//  ui->tabWidget->setTabEnabled(3,false);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_actionExit_triggered() {
  close();
}

bool MainWindow::save() {
  QString fname = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath(), tr("Panda files (*.panda)"));
  if( fname.isEmpty() ) {
    return false;
  }
  QFile fl(fname);
  if( fl.open(QFile::WriteOnly) ) {
    QDataStream ds( &fl );
    try{
      editor->save(ds);
    } catch ( std::runtime_error &e ) {
      std::cerr << "Error saving project: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "Could not open file in WriteOnly mode : " << fname.toStdString() << "." << std::endl;
    return false;
  }
  return true;
}

void MainWindow::on_actionNew_triggered() {
  QMessageBox msgBox;
  msgBox.setParent(this);
  msgBox.setLocale(QLocale::Portuguese);
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  msgBox.setText(tr("Do you want to save your changes?"));
  msgBox.setWindowModality(Qt::WindowModal);
  msgBox.setDefaultButton(QMessageBox::Save);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Save) {
    if(save()) {
      editor->clear();
    }
  } else if( ret == QMessageBox::Discard) {
    editor->clear();
  } else if (ret == QMessageBox::Cancel) {
    return;
  }
}

void MainWindow::on_actionWires_triggered(bool checked) {
  editor->showWires(checked);
}

void MainWindow::on_actionRotate_right_triggered() {
  editor->rotate(true);
}

void MainWindow::on_actionRotate_left_triggered() {
  editor->rotate(false);
}

void MainWindow::on_actionOpen_triggered() {
  QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(),tr("Panda files (*.panda)"));
  if( fname.isEmpty() ) {
    return;
  }
  QFile fl(fname);
  if( !fl.exists() ) {
    std::cerr << "Error: This file does not exists: " << fname.toStdString() << std::endl;
    return;
  }
  if( fl.open(QFile::ReadOnly) ) {
    QDataStream ds( &fl );
    try{
      editor->load(ds);
    } catch ( std::runtime_error &e ) {
      std::cerr << "Error loading project: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "Could not open file in ReadOnly mode : " << fname.toStdString() << "." << std::endl;
    return;
  }
  return;
}

void MainWindow::on_actionSave_triggered() {
  save();
}

void MainWindow::on_actionAbout_triggered() {
  QMessageBox::about(this, "Wired Panda", tr("Wired Panda is a software built to help students to learn about logic circuits.\n\nCreators:\nDavi Morales\nHéctor Castelli\nLucas Lellis\nRodrigo Torres\nSupervised by: Fábio Cappabianco."));
}

void MainWindow::on_actionAbout_Qt_triggered() {
  QMessageBox::aboutQt(this);
}

void MainWindow::on_actionDelete_triggered() {
  editor->deleteElements();
}

void MainWindow::on_lineEdit_textEdited(const QString &) {

}

void MainWindow::closeEvent(QCloseEvent * e) {
  QMessageBox msgBox;
  msgBox.setParent(this);
  msgBox.setLocale(QLocale::Portuguese);
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  msgBox.setText(tr("Do you want to save your changes?"));
  msgBox.setWindowModality(Qt::WindowModal);
  msgBox.setDefaultButton(QMessageBox::Save);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Save) {
    if(save()) {
      close();
    }
  } else if( ret == QMessageBox::Discard) {
    close();
  } else if (ret == QMessageBox::Cancel) {
    e->ignore();
  }
}
