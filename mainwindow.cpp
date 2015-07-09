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
  scene = new Scene(this);
  scene->setBackgroundBrush(QBrush(QColor(Qt::gray)));
  scene->setGridSize(16);
  ui->graphicsView->setScene(scene);
//  ui->graphicsView->setBackgroundBrush(QBrush(QColor(Qt::gray)));
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
  if(!fname.endsWith(".panda")){
    fname.append(".panda");
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

void MainWindow::show() {
  QMainWindow::show();
  editor->clear();
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
      QMessageBox::warning(this,"Error!","Could not open file.\nError: " + QString(e.what()),QMessageBox::Ok,QMessageBox::NoButton);
      clear();
      return;
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
  QMessageBox::about(this, "Wired Panda", tr("Wired Panda is a software built to help students to learn about logic circuits.\nVersion: %1\n\nCreators:\nDavi Morales\nHéctor Castelli\nLucas Lellis\nRodrigo Torres\nSupervised by: Fábio Cappabianco.").arg(QApplication::applicationVersion()));
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
#ifdef DEBUG
  return;
#endif
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
  if(!fname.endsWith(".panda")){
    fname.append(".panda");
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
  if(value.fileName().isEmpty()) {
    setWindowTitle("WiRED PANDA");
  } else {
    setWindowTitle(QString("WiRED PANDA ( %1 )").arg(value.fileName()));
  }
  if(currentFile.exists()) {
    defaultDirectory = currentFile.dir();
  } else {
    defaultDirectory = QDir::home();
  }
}

void MainWindow::on_actionSelect_all_triggered() {
  editor->selectAll();
}

void MainWindow::on_actionOpen_Box_triggered() {
  QString fname = QFileDialog::getOpenFileName(this, tr("Open File as Box"), defaultDirectory.absolutePath(),tr("Panda files (*.panda)"));
  if( fname.isEmpty() ) {
    return;
  }
  QFile fl(fname);
  if( !fl.exists() ) {
    std::cerr << "Error: This file does not exists: " << fname.toStdString() << std::endl;
    return;
  }
  if( fl.open(QFile::ReadOnly) ) {
    try {
      QHBoxLayout * horizontalLayout_BOX = new QHBoxLayout();
      horizontalLayout_BOX->setSpacing(6);
      horizontalLayout_BOX->setObjectName(QStringLiteral("horizontalLayout_BOX"));
      horizontalLayout_BOX->setSizeConstraint(QLayout::SetFixedSize);
      QPixmap pixmap(QString::fromUtf8(":/basic/resources/basic/box.svg"));
      Label * label_BOX_ICON = new Label(ui->scrollAreaWidgetContents_Box);
      label_BOX_ICON->setObjectName(QStringLiteral("label_box"));
      label_BOX_ICON->setPixmap(pixmap);
      label_BOX_ICON->setAuxData(fname);

      horizontalLayout_BOX->addWidget(label_BOX_ICON);

      QLabel * label_BOX = new QLabel(ui->scrollAreaWidgetContents_Box);
      label_BOX->setObjectName(QStringLiteral("label_BOX"));
      label_BOX->setText(QFileInfo(fname).baseName().toUpper());
      horizontalLayout_BOX->addWidget(label_BOX);
      ui->verticalLayout_4->removeItem(ui->verticalSpacer_BOX);
      ui->verticalLayout_4->addLayout(horizontalLayout_BOX);
      ui->verticalLayout_4->addItem(ui->verticalSpacer_BOX);
    } catch ( std::runtime_error &e ) {
      std::cerr << "Error loading project as box: " << e.what() << std::endl;
      QMessageBox::warning(this,"Error!","Could not open file.\nError: " + QString(e.what()),QMessageBox::Ok,QMessageBox::NoButton);
      clear();
      return;
    }
  } else {
    std::cerr << "Could not open file in ReadOnly mode : " << fname.toStdString() << "." << std::endl;
    return;
  }
  fl.close();
  ui->statusBar->showMessage("Loaded box sucessfully.",2000);
}
