// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ICDropZone.h"

#include <optional>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QLabel>
#include <QMimeData>
#include <QResizeEvent>

#include "App/Core/DragDropPayload.h"
#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"

namespace {

std::optional<DragDropPayload> extractDragPayload(const QMimeData *mimeData)
{
    QByteArray itemData;
    if (mimeData->hasFormat(MimeType::DragDrop)) {
        itemData = mimeData->data(MimeType::DragDrop);
    } else if (mimeData->hasFormat(MimeType::DragDropLegacy)) {
        itemData = mimeData->data(MimeType::DragDropLegacy);
    } else {
        return std::nullopt;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    return readDragDropPayload(stream);
}

} // namespace

ICDropZone::ICDropZone(Section section, QWidget *parent)
    : QWidget(parent)
    , m_section(section)
{
    setAcceptDrops(true);

    // Cross-section drag-to-convert is otherwise invisible. A raised overlay, shown only
    // while a compatible IC is dragged over, spells out what the drop will do.
    m_hintOverlay = new QLabel(this);
    m_hintOverlay->setObjectName("icDropHint");
    m_hintOverlay->setAlignment(Qt::AlignCenter);
    m_hintOverlay->setWordWrap(true);
    m_hintOverlay->setText(m_section == Section::Embedded
                               ? tr("Drop here to embed this IC in the circuit")
                               : tr("Drop here to extract this IC to a file"));
    // Concrete rgba (Qt Style Sheets don't support palette()); blue+white reads on both themes.
    m_hintOverlay->setStyleSheet(QStringLiteral(
        "QLabel { background: rgba(51, 129, 204, 215); color: white;"
        " border: 2px dashed white; border-radius: 6px; padding: 8px; font-weight: bold; }"));
    m_hintOverlay->hide();
}

void ICDropZone::setHintVisible(bool visible)
{
    if (visible) {
        m_hintOverlay->resize(size());
        m_hintOverlay->raise();
    }
    m_hintOverlay->setVisible(visible);
}

void ICDropZone::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_hintOverlay->resize(event->size());
}

void ICDropZone::dragEnterEvent(QDragEnterEvent *event)
{
    auto payload = extractDragPayload(event->mimeData());
    if (!payload) return;

    // Only accept opposite-type drops
    if (m_section == Section::Embedded && !payload->isEmbedded) {
        event->acceptProposedAction(); // File-based dropped onto embedded section
        setHintVisible(true);
    } else if (m_section == Section::FileBased && payload->isEmbedded) {
        event->acceptProposedAction(); // Embedded dropped onto file-based section
        setHintVisible(true);
    }
}

void ICDropZone::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void ICDropZone::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    setHintVisible(false);
}

void ICDropZone::dropEvent(QDropEvent *event)
{
    setHintVisible(false);

    auto payload = extractDragPayload(event->mimeData());
    if (!payload) return;

    if (m_section == Section::Embedded && !payload->isEmbedded) {
        emit embedByFileRequested(payload->icFileName);
        event->acceptProposedAction();
    } else if (m_section == Section::FileBased && payload->isEmbedded) {
        emit extractByBlobNameRequested(payload->blobName);
        event->acceptProposedAction();
    }
}
