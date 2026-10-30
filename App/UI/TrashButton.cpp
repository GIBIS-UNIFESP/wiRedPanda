// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/TrashButton.h"

#include <QDragEnterEvent>
#include <QMessageBox>
#include <QMimeData>

#include "App/Core/DragDropPayload.h"
#include "App/Core/Enums.h"

TrashButton::TrashButton(QWidget *parent)
    : QPushButton(parent)
{
    setAcceptDrops(true);
}

void TrashButton::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept both the legacy MIME type and the current one so that IC files dragged from either old or new element
    // panels are accepted; a payload from another program that does not read is refused
    if (readDragDropPayload(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void TrashButton::dropEvent(QDropEvent *event)
{
    // The IC file name and the optional embedded fields; a payload that does not read is refused
    if (const auto payload = readDragDropPayload(event->mimeData())) {

        QMessageBox msgBox;
        msgBox.setParent(this);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setDefaultButton(QMessageBox::No);

        if (payload->isEmbedded) {
            msgBox.setText(tr("Remove all \"%1\" instances from the circuit?").arg(payload->blobName));
        } else {
            msgBox.setText(tr("Remove this IC? Its file will be moved to the system trash and its instances deleted from the circuit."));
        }

        if (msgBox.exec() != QMessageBox::Yes) {
            event->setAccepted(false);
            return;
        }

        if (payload->isEmbedded) {
            emit removeEmbeddedIC(payload->blobName);
        } else {
            emit removeICFile(payload->icFileName);
        }
    }
}
