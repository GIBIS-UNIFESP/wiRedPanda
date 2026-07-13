// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/InlineLabelEditor.h"

#include <algorithm>

#include <QGraphicsProxyWidget>
#include <QKeyEvent>
#include <QLineEdit>

#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"

namespace {

/// Adds Escape-to-cancel on top of QLineEdit's existing returnPressed()/editingFinished()
/// (Enter and focus-out already commit through those signals -- see start()).
class InlineLabelLineEdit : public QLineEdit
{
public:
    explicit InlineLabelLineEdit(InlineLabelEditor *owner)
        : m_owner(owner)
    {
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Escape) {
            m_owner->cancel();
            return;
        }

        QLineEdit::keyPressEvent(event);
    }

private:
    InlineLabelEditor *m_owner;
};

} // namespace

InlineLabelEditor::InlineLabelEditor(Scene *scene)
    : m_scene(scene)
{
}

InlineLabelEditor::~InlineLabelEditor() = default;

void InlineLabelEditor::start(GraphicElement *element)
{
    if (!element) {
        return;
    }

    // Only one inline edit at a time; committing here also handles the common case of
    // double-clicking a different element while this one is still focused (which itself
    // already lost focus and committed via editingFinished by the time the double-click's
    // second press lands, but this stays correct even if that assumption ever changes).
    if (m_target) {
        commit();
    }

    m_target = element;

    auto *lineEdit = new InlineLabelLineEdit(this);
    lineEdit->setText(element->label());

    QObject::connect(lineEdit, &QLineEdit::returnPressed, lineEdit, [this] { commit(); });
    QObject::connect(lineEdit, &QLineEdit::editingFinished, lineEdit, [this] { commit(); });

    m_proxy = m_scene->addWidget(lineEdit);
    m_proxy->setZValue(1000); // above every element/wire on the canvas

    QRectF targetRect = element->labelSceneBoundingRect();
    if (targetRect.isEmpty()) {
        targetRect = element->sceneBoundingRect();
    }
    const qreal width = std::max(targetRect.width(), 80.0);
    lineEdit->resize(static_cast<int>(width), lineEdit->sizeHint().height());
    m_proxy->setPos(targetRect.topLeft());

    lineEdit->setFocus();
    lineEdit->selectAll();
}

void InlineLabelEditor::commit()
{
    if (!m_target) {
        return;
    }

    auto *lineEdit = qobject_cast<QLineEdit *>(m_proxy->widget());
    const QString newText = lineEdit ? lineEdit->text() : m_target->label();

    if (newText != m_target->label()) {
        QByteArray oldData;
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        m_target->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});

        m_target->setLabel(newText);
        m_scene->receiveCommand(new UpdateCommand({m_target}, oldData, m_scene));
    }

    close();
}

void InlineLabelEditor::cancel()
{
    close();
}

void InlineLabelEditor::close()
{
    m_target = nullptr;
    if (m_proxy) {
        m_proxy->deleteLater();
        m_proxy = nullptr;
    }
}
