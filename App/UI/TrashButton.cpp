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
        QByteArray itemData;

        if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
            itemData = event->mimeData()->data("wpanda/x-dnditemdata");
        }

        if (event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
            itemData = event->mimeData()->data("application/x-wiredpanda-dragdrop");
        }

        // Deserialise the drag payload to extract the IC file name and optional embedded fields
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        Serialization::readPandaHeader(stream);

        QPoint offset;      stream >> offset;
        ElementType type;   stream >> type;
        QString icFileName; stream >> icFileName;

        bool isEmbedded = false;
        QString blobName;
        if (!stream.atEnd()) { stream >> isEmbedded; }
        if (!stream.atEnd()) { stream >> blobName; }

        QMessageBox msgBox;
        msgBox.setParent(this);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setDefaultButton(QMessageBox::No);

        if (isEmbedded) {
            msgBox.setText(tr("Remove all \"%1\" instances from the circuit?").arg(blobName));
        } else {
            msgBox.setText(tr("File will be deleted. Are you sure?"));
        }

        if (msgBox.exec() != QMessageBox::Yes) {
            event->setAccepted(false);
            return;
        }

        if (isEmbedded) {
            emit removeEmbeddedIC(blobName);
        } else {
            emit removeICFile(icFileName);
        }
    }
}

