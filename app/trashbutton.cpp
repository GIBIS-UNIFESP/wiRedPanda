#include "trashbutton.h"

#include "enums.h"
#include "serialization.h"

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
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")
        || event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
        event->acceptProposedAction();
    }
}

void TrashButton::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")
        || event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
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

        QByteArray itemData;

        if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
            itemData = event->mimeData()->data("wpanda/x-dnditemdata");
        }

        if (event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
            itemData = event->mimeData()->data("application/x-wiredpanda-dragdrop");
        }

        QDataStream stream(&itemData, QIODevice::ReadOnly);
        Serialization::readPandaHeader(stream);

        QPoint offset;      stream >> offset;
        ElementType type;   stream >> type;
        QString icFileName; stream >> icFileName;

        emit removeGlobalICFile(icFileName);
    }
}
