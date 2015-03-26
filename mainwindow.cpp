#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

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
  scene->setSceneRect(ui->graphicsView->rect());
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_actionExit_triggered() {
  close();
}

void MainWindow::on_actionNew_triggered() {
  editor->clear();
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
  QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(),tr("Panda files (*.panda)"));
}

void MainWindow::on_actionSave_triggered() {
  QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath(), tr("Panda files (*.panda)"));
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

void MainWindow::on_lineEdit_textEdited(const QString &arg1) {

}
