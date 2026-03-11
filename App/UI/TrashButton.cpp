// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/TrashButton.h"

#include <QDragEnterEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QVersionNumber>

#include "App/Core/Enums.h"
#include "App/IO/Serialization.h"

TrashButton::TrashButton(QWidget *parent)
    : QPushButton(parent)
{
    setAcceptDrops(true);
}

void TrashButton::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept both the legacy MIME type ("wpanda/x-dnditemdata") and the current one
    // so that IC files dragged from either old or new element panels are accepted
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")
        || event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
        event->acceptProposedAction();
    }
}

void TrashButton::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")
        || event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
        // Require explicit confirmation before permanently deleting an IC file
        QMessageBox msgBox;
        msgBox.setParent(this);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText(tr("File will be deleted. Are you sure?"));
        msgBox.setWindowModality(Qt::WindowModal);
        // Default to "No" so an accidental drag does not cause data loss
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() != QMessageBox::Yes) {
            event->setAccepted(false);
            return;
        }

        QByteArray itemData;

        // Prefer the newer MIME type but accept either; the second assignment wins
        // if both formats are somehow present (shouldn't happen in practice)
        if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
            itemData = event->mimeData()->data("wpanda/x-dnditemdata");
        }

        if (event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
            itemData = event->mimeData()->data("application/x-wiredpanda-dragdrop");
        }

        // Deserialise the drag payload to extract only the IC file name;
        // offset and type are also present in the stream but not needed here
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        Serialization::readPandaHeader(stream);

        QPoint offset;      stream >> offset;
        ElementType type;   stream >> type;
        QString icFileName; stream >> icFileName;

        emit removeICFile(icFileName);
    }
}

