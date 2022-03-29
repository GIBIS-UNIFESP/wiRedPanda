#include "application.h"

#include <QDebug>
#include <QMessageBox>

Application::Application(int &argc, char **argv) : QApplication(argc, argv) {
}

bool Application::notify(QObject *receiver, QEvent *event) {
    bool done = true;

    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "Erro", e.what());
    }

    return done;
}
