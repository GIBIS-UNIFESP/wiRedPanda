// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickDialogProvider.h"

#include <memory>

#include <QEventLoop>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QVariant>
#include <QVariantList>

namespace {

/// Opens \a dialog and blocks in a nested event loop until it emits accepted() or
/// rejected() -- both QtQuick.Dialogs' MessageDialog (via QQuickAbstractDialog) and
/// QtQuick.Controls' Dialog (via Popup) expose these same two signal names, so one helper
/// covers both ChoiceDialog.qml and TextPromptDialog.qml. Mirrors QDialog::exec()'s own
/// well-established nested-event-loop pattern.
void execModal(QObject *dialog)
{
    QEventLoop loop;
    QObject::connect(dialog, SIGNAL(accepted()), &loop, SLOT(quit()));
    QObject::connect(dialog, SIGNAL(rejected()), &loop, SLOT(quit()));
    QMetaObject::invokeMethod(dialog, "open");
    loop.exec();
}

} // namespace

QuickDialogProvider::QuickDialogProvider(QQuickWindow *rootWindow)
    : m_rootWindow(rootWindow)
    , m_choiceComponent(qmlEngine(rootWindow), QUrl(QStringLiteral("qrc:/qt/qml/QuickShell/ChoiceDialog.qml")))
    , m_textPromptComponent(qmlEngine(rootWindow), QUrl(QStringLiteral("qrc:/qt/qml/QuickShell/TextPromptDialog.qml")))
{
}

DialogButton QuickDialogProvider::choice(const QString &title, const QString &text,
                                          const QList<DialogButton> &buttons, DialogButton defaultButton)
{
    std::unique_ptr<QObject> dialog(m_choiceComponent.create(qmlContext(m_rootWindow)));
    Q_ASSERT(dialog);

    QVariantList requested;
    requested.reserve(buttons.size());
    for (DialogButton button : buttons) {
        requested.append(static_cast<int>(button));
    }

    dialog->setProperty("parentWindow", QVariant::fromValue(static_cast<QWindow *>(m_rootWindow)));
    dialog->setProperty("title", title);
    dialog->setProperty("text", text);
    dialog->setProperty("requestedButtons", requested);

    emit dialogOpened(dialog.get());
    execModal(dialog.get());

    const int resultIndex = dialog->property("resultIndex").toInt();
    if (resultIndex < 0) {
        return defaultButton;
    }
    return static_cast<DialogButton>(resultIndex);
}

std::optional<QString> QuickDialogProvider::textPrompt(const QString &title, const QString &label,
                                                         const QString &initialValue)
{
    std::unique_ptr<QObject> dialog(m_textPromptComponent.create(qmlContext(m_rootWindow)));
    Q_ASSERT(dialog);

    dialog->setProperty("parent", QVariant::fromValue(m_rootWindow->contentItem()));
    dialog->setProperty("title", title);
    dialog->setProperty("promptLabel", label);
    dialog->setProperty("value", initialValue);

    emit dialogOpened(dialog.get());
    execModal(dialog.get());

    if (!dialog->property("accepted_").toBool()) {
        return std::nullopt;
    }
    return dialog->property("value").toString();
}
