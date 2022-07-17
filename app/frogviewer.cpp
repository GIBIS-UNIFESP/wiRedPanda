#include "frogviewer.h"
#include "ui_frogviewer.h"

#include "graphicelementinput.h"


#include <QDebug>
#include <QSaveFile>
#include <QFileDialog>

FrogViewer::FrogViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrogViewer)
{
    ui->setupUi(this);
    qDebug() <<  "construtor do frogviewer";

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


void FrogViewer::on_previous_button_clicked()
{

}

void FrogViewer::processaSinal(GraphicElementInput *elm){
    // verifica o tipo do elemento, ex: botao, rotativo, interruptor
    // verifica para qual estado ele foi, ex: foi para ligado/desligado, rotativo girou
    // insere na tabela o elemento e o estado alterado

    qDebug() << "clicked elm: " << elm;
}

void FrogViewer::on_rec_button_clicked()
{
    if(ui->rec_button->isChecked()){
//        connect(m_currentTab->scene(), &Scene::clickedElement, this, &FrogViewer::processaSinal);
    } else{
//        disconnect(m_currentTab->scene(), &Scene::clickedElement, this, &FrogViewer::processaSinal);
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


void FrogViewer::on_close_button_clicked()
{
    this->hide();
    if(ui->rec_button->isChecked()){
        ui->rec_button->click();
    }

}

void FrogViewer::on_next_button_clicked()
{
    // para salvar os estados dos elementos usar BewavedDolphin::restoreInputs
    // e loadSignals

    qDebug() <<  "next: " << m_currentTab->scene()->elements();
    m_currentTab->simulation()->update();
}

void FrogViewer::on_gif_button_clicked()
{
    if (!m_currentTab) {
        return;
    }

    QString path;

    //if (m_currentFile.exists()) {
     //   path = m_currentFile.absolutePath();
    //}

    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to GIF"), path, tr("PNG files (*.png)"));
    QString pngFilebkp = pngFile;
    QString file_name = pngFilebkp.remove(0, pngFile.lastIndexOf("/")+1);
    for(int i=0; i < 30; ++i ){
        qDebug() << file_name;

        pngFile.replace(pngFile.lastIndexOf("/")+1, (pngFile.size() - pngFile.lastIndexOf("/")+1), file_name+"_"+QString::number(i)+".png");

        if (pngFile.isEmpty()) {
            return;
        }

        if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
            pngFile.append(".png");
        }

        QRectF rect = m_currentTab->scene()->itemsBoundingRect().adjusted(-64, -64, 64, 64);
        QPixmap pixmap(rect.size().toSize());
        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        m_currentTab->scene()->render(&painter, QRectF(), rect);
        painter.end();
        pixmap.save(pngFile);
        m_currentTab->simulation()->update();
        qDebug() << pngFile;
    }
//    ->statusBar->showMessage(tr("Exported file successfully."), 4000);
    //dynamic_cast<MainWindow *>(parentWidget());
}

