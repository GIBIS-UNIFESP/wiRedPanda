// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frogviewer.h"
#include "ui_frogviewer.h"

#include "gif.h"
#include "gifwriter.h"
#include "graphicelementinput.h"

#include <QDebug>
#include <QFileDialog>
#include <QSaveFile>

FrogViewer::FrogViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FrogViewer)
{
    ui->setupUi(this);
    qDebug() <<  "construtor do frogviewer";

    connect(ui->close_button,    &QPushButton::clicked, this, &FrogViewer::on_closeButton_clicked);
    connect(ui->gif_button,      &QPushButton::clicked, this, &FrogViewer::on_gifButton_clicked);
    connect(ui->next_button,     &QPushButton::clicked, this, &FrogViewer::on_nextButton_clicked);
    connect(ui->previous_button, &QPushButton::clicked, this, &FrogViewer::on_previousButton_clicked);
    connect(ui->rec_button,      &QPushButton::clicked, this, &FrogViewer::on_recButton_clicked);
}

FrogViewer::~FrogViewer()
{
    qDebug() <<  "destrutor do frogviewer";
    delete ui;
}

void FrogViewer::setTab(WorkSpace *newTab)
{
    m_currentTab = newTab;
   // m_currentFile = m_currentTab->fileInfo();
}

// aqui eh so criar um slot para cada botao do forms
// do frog e acessar os elementos usando 'm_tab'

void FrogViewer::on_previousButton_clicked()
{

}

void FrogViewer::processSignal(GraphicElementInput *elm){
    // verifica o tipo do elemento, ex: botao, rotativo, interruptor
    // verifica para qual estado ele foi, ex: foi para ligado/desligado, rotativo girou
    // insere na tabela o elemento e o estado alterado

    qDebug() << "clicked elm: " << elm;
}

void FrogViewer::on_recButton_clicked()
{
    if (!m_currentTab) {
        qDebug() << "no tab";
        return;
    }

    if(ui->rec_button->isChecked()){
        connect(m_currentTab->scene(), &Scene::clickedElement, this, &FrogViewer::processSignal);
    } else{
        disconnect(m_currentTab->scene(), &Scene::clickedElement, this, &FrogViewer::processSignal);
    }
    //    QVector<int> vetor = { 1, 0, 2 };
//    (vetor << 1) << 2;
//    vetor << 1;
//    vetor << 2;
//    vetor.append(1);
//    vetor[0] = 1;

//    auto elements = m_currentTab->scene()->elements();
//    qDebug() << m_currentTab->scene()->elements();

//    for(int i=0; i < elements.size(); ++i )
//    for(auto i : elements){

//    }

//    Qvector<int> nome = m_tab->scene()->elements();
}

void FrogViewer::on_closeButton_clicked()
{
    this->hide();
    if(ui->rec_button->isChecked()){
        ui->rec_button->click();
    }

}

void FrogViewer::on_nextButton_clicked()
{
    // para salvar os estados dos elementos usar BewavedDolphin::restoreInputs
    // e loadSignals

    qDebug() <<  "next: " << m_currentTab->scene()->elements();
    m_currentTab->simulation()->update();
}

void FrogViewer::on_gifButton_clicked()
{
    if (!m_currentTab) {
        return;
    }

    QString path;

    //if (m_currentFile.exists()) {
     //   path = m_currentFile.absolutePath();
    //}

    QString filePath = QFileDialog::getSaveFileName(this, tr("Export to GIF"), path, tr("PNG files (*.png)"));

    if (filePath.isEmpty()) {
        return;
    }

    if (!filePath.endsWith(".gif")) {
        filePath += ".gif";
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

//    QString pngFilebkp = filePath;
//    QString file_name = pngFilebkp.remove(0, filePath.lastIndexOf("/")+1);

    std::vector<QImage> imgs;

    for(int i=0; i < 30; ++i ){
        qDebug() << fileName;

        fileName = fileInfo.baseName() + "_" + QString::number(i) + ".png";
//        filePath.replace(filePath.lastIndexOf("/")+1, (filePath.size() - filePath.lastIndexOf("/")+1), file_name+"_"+QString::number(i)+".png");

        if (filePath.isEmpty()) {
            return;
        }

        if (!filePath.endsWith(".png", Qt::CaseInsensitive)) {
            filePath.append(".png");
        }

        QRectF rect = m_currentTab->scene()->itemsBoundingRect().adjusted(-64, -64, 64, 64);
        QPixmap pixmap(rect.size().toSize());
        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        m_currentTab->scene()->render(&painter, QRectF(), rect);
        painter.end();

        imgs.push_back(pixmap.toImage());

//        pixmap.save(pngFile);
        m_currentTab->simulation()->update();
//        qDebug() << filePath;
    }

    int delay = 20;
    bool dither = false;

    QFile file(fileInfo.absoluteFilePath());

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "couldn't open file for writing: " + file.errorString();
        return;
    }

    file.write(MyLib::GifWriter::encode(imgs, delay, dither));
    file.close();

//    ->statusBar->showMessage(tr("Exported file successfully."), 4000);
    //dynamic_cast<MainWindow *>(parentWidget());
}
