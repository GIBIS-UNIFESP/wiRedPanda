#include "application.h"

#include <QDebug>
#include <QMessageBox>

Application::Application(int &argc, char **argv) : QApplication(argc, argv) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setAttribute(Qt::AA_EnableHighDpiScaling);
    setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
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
