#include "trashbutton.h"

#include "enums.h"

#include <QDragEnterEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QVersionNumber>

TrashButton::TrashButton(QWidget *parent)
    : QPushButton(parent)
{
    setAcceptDrops(true);
}

void TrashButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
        event->acceptProposedAction();
    }
}

void TrashButton::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
        QMessageBox msgBox;
        msgBox.setParent(this);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText(tr("File will be deleted. Are you sure?"));
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() != QMessageBox::Yes) {
            event->setAccepted(false);
            return;
        }

        QByteArray itemData = event->mimeData()->data("wpanda/x-dnditemdata");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_12);

        QVersionNumber version;
        qint64 originalPos = stream.device()->pos();

        try {
            stream >> version;
        } catch (std::bad_alloc &) {
            // before 4.2 no QVersionNumber were set in the stream
            version = QVersionNumber(4, 1);
            stream.device()->seek(originalPos);
        }

        QPoint offset;      stream >> offset;
        ElementType type;   stream >> type;
        QString icFileName; stream >> icFileName;

        emit removeICFile(icFileName);
    }
}
