#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMimeData>
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
  setCurrentFile(QFileInfo());

  connect(editor,&Editor::scroll,this,&MainWindow::scrollView);
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
  QString fname = currentFile.absoluteFilePath();
  if(!currentFile.exists()) {
    fname = QFileDialog::getSaveFileName(this, tr("Save File"), defaultDirectory.absolutePath(), tr("Panda files (*.panda)"));
  }
  if( fname.isEmpty() ) {
    return false;
  }
  QFile fl(fname);
  if( fl.open(QFile::WriteOnly) ) {
    QDataStream ds( &fl );
    try {
      editor->save(ds);
    } catch ( std::runtime_error &e ) {
      std::cerr << "Error saving project: " << e.what() << std::endl;
      return false;
    }
  } else {
    std::cerr << "Could not open file in WriteOnly mode : " << fname.toStdString() << "." << std::endl;
    return false;
  }
  fl.close();
  setCurrentFile(QFileInfo(fname));
  ui->statusBar->showMessage("Saved file sucessfully.",2000);
  return true;
}

void MainWindow::clear() {
  editor->clear();
  setCurrentFile(QFileInfo());
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
      clear();
    }
  } else if( ret == QMessageBox::Discard) {
    clear();
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

void MainWindow::open(const QString &fname ) {
  QFile fl(fname);
  if( !fl.exists() ) {
    std::cerr << "Error: This file does not exists: " << fname.toStdString() << std::endl;
    return;
  }
  if( fl.open(QFile::ReadOnly) ) {
    QDataStream ds( &fl );
    try {
      editor->load(ds);
    } catch ( std::runtime_error &e ) {
      std::cerr << "Error loading project: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "Could not open file in ReadOnly mode : " << fname.toStdString() << "." << std::endl;
    return;
  }
  fl.close();
  setCurrentFile(QFileInfo(fname));
  ui->statusBar->showMessage("Loaded file sucessfully.",2000);
}

void MainWindow::scrollView(int dx, int dy) {
  ui->graphicsView->scroll(dx, dy);
}

void MainWindow::on_actionOpen_triggered() {
  QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), defaultDirectory.absolutePath(),tr("Panda files (*.panda)"));
  if( fname.isEmpty() ) {
    return;
  }
  open(fname);
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

void MainWindow::on_actionCopy_triggered() {
  QClipboard *clipboard = QApplication::clipboard();
  QMimeData *mimeData = new QMimeData;
  QByteArray itemData;
  QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  editor->copy(dataStream);
  mimeData->setData("application/copydata", itemData);
  clipboard->setMimeData(mimeData);
}

void MainWindow::on_actionCut_triggered() {
  QClipboard *clipboard = QApplication::clipboard();
  QMimeData *mimeData = new QMimeData;
  QByteArray itemData;
  QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  editor->cut(dataStream);
  mimeData->setData("application/copydata", itemData);
  clipboard->setMimeData(mimeData);
}

void MainWindow::on_actionPaste_triggered() {
  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  if(mimeData->hasFormat("application/copydata")) {
    QByteArray itemData = mimeData->data("application/copydata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    editor->paste(dataStream);
  }
}

void MainWindow::on_actionSave_As_triggered() {
  QString fname = currentFile.absoluteFilePath();
  fname = QFileDialog::getSaveFileName(this, tr("Save File as ..."), defaultDirectory.absolutePath(), tr("Panda files (*.panda)"));
  if( fname.isEmpty() ) {
    return;
  }
  QFile fl(fname);
  if( fl.open(QFile::WriteOnly) ) {
    QDataStream ds( &fl );
    try {
      editor->save(ds);
    } catch ( std::runtime_error &e ) {
      std::cerr << "Error saving project: " << e.what() << std::endl;
      return;
    }
  } else {
    std::cerr << "Could not open file in WriteOnly mode : " << fname.toStdString() << "." << std::endl;
    return;
  }
  fl.close();
  ui->statusBar->showMessage("Saved file sucessfully.",2000);
  setCurrentFile(QFileInfo(fname));
  return;
}

QFileInfo MainWindow::getCurrentFile() const {
  return currentFile;
}

void MainWindow::setCurrentFile(const QFileInfo & value) {
  qDebug() << "Setting current file to: " << value.absoluteFilePath();
  currentFile = value;
  if(currentFile.exists()) {
    defaultDirectory = currentFile.dir();
  } else {
    defaultDirectory = QDir::home();
  }
}

void MainWindow::on_actionSelect_all_triggered() {
  editor->selectAll();
}
