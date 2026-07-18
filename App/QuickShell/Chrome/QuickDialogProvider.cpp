// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickDialogProvider.h"

#include <memory>

#include <QEventLoop>
#include <QFileInfo>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QUrl>
#include <QVariant>
#include <QVariantList>

namespace {

/// Opens \a dialog and blocks in a nested event loop until it emits accepted() or
/// rejected() -- both QtQuick.Dialogs' MessageDialog/FileDialog (via QQuickAbstractDialog)
/// and QtQuick.Controls' Dialog (via Popup) expose these same two signal names, so one helper
/// covers ChoiceDialog.qml, TextPromptDialog.qml, and QuickFileDialog.qml alike. Mirrors
/// QDialog::exec()'s own well-established nested-event-loop pattern.
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
    , m_fileDialogComponent(qmlEngine(rootWindow), QUrl(QStringLiteral("qrc:/qt/qml/QuickShell/QuickFileDialog.qml")))
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

QString QuickDialogProvider::getOpenFileName(QWidget * /*parent*/, const QString &caption,
                                              const QString &dir, const QString &filter)
{
    std::unique_ptr<QObject> dialog(m_fileDialogComponent.create(qmlContext(m_rootWindow)));
    Q_ASSERT(dialog);

    dialog->setProperty("parentWindow", QVariant::fromValue(static_cast<QWindow *>(m_rootWindow)));
    dialog->setProperty("title", caption);
    dialog->setProperty("fileMode", 0); // FileDialog.OpenFile -- see QuickFileDialog.qml
    if (!dir.isEmpty()) {
        dialog->setProperty("currentFolder", QUrl::fromLocalFile(dir));
    }
    if (!filter.isEmpty()) {
        dialog->setProperty("nameFilters", QVariant::fromValue(QStringList{filter}));
    }

    emit dialogOpened(dialog.get());
    execModal(dialog.get());

    if (!dialog->property("accepted_").toBool()) {
        return {};
    }
    return dialog->property("selectedFile").toUrl().toLocalFile();
}

FileDialogResult QuickDialogProvider::getSaveFileName(QWidget * /*parent*/, const QString &caption,
                                                        const QString &dir, const QString &filter)
{
    std::unique_ptr<QObject> dialog(m_fileDialogComponent.create(qmlContext(m_rootWindow)));
    Q_ASSERT(dialog);

    dialog->setProperty("parentWindow", QVariant::fromValue(static_cast<QWindow *>(m_rootWindow)));
    dialog->setProperty("title", caption);
    dialog->setProperty("fileMode", 2); // FileDialog.SaveFile -- see QuickFileDialog.qml
    if (!dir.isEmpty()) {
        // dir may be a bare directory or a full file path (both are real call shapes in
        // WorkspaceManager.cpp, matching QFileDialog::getSaveFileName()'s own dir semantics)
        // -- QFileInfo::absolutePath() alone would wrongly treat a bare directory's last path
        // component as a filename, so branch on whether it actually is one.
        const QFileInfo dirInfo(dir);
        if (dirInfo.isDir()) {
            dialog->setProperty("currentFolder", QUrl::fromLocalFile(dirInfo.absoluteFilePath()));
        } else {
            dialog->setProperty("currentFolder", QUrl::fromLocalFile(dirInfo.absolutePath()));
            dialog->setProperty("selectedFile", QUrl::fromLocalFile(dirInfo.absoluteFilePath()));
        }
    }
    if (!filter.isEmpty()) {
        dialog->setProperty("nameFilters", QVariant::fromValue(QStringList{filter}));
    }

    emit dialogOpened(dialog.get());
    execModal(dialog.get());

    if (!dialog->property("accepted_").toBool()) {
        return {};
    }

    QString selectedFilter;
    if (auto *filterObj = dialog->property("selectedNameFilter").value<QObject *>()) {
        selectedFilter = filterObj->property("name").toString();
    }
    return {dialog->property("selectedFile").toUrl().toLocalFile(), selectedFilter};
}
