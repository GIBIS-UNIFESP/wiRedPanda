
#include "workspace.h"
#ifndef FROGVIEWER_H
#define FROGVIEWER_H

#include <QWidget>

namespace Ui {
class FrogViewer;
}

class FrogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit FrogViewer(QWidget *parent = nullptr);
    ~FrogViewer();

    void setTab(WorkSpace *newTab);

private slots:


    void on_previous_button_clicked();

    void on_rec_button_clicked();

    void on_close_button_clicked();

    void on_next_button_clicked();

    void on_gif_button_clicked();

private:
    Ui::FrogViewer *ui;
    WorkSpace *m_currentTab;
    void processaSinal(GraphicElementInput *elm);
};

#endif // FROGVIEWER_H
