// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickDialogProvider: DialogProvider backed by QML dialogs (QtQuick.Dialogs'
 * MessageDialog and a small TextPromptDialog.qml component).
 */

#pragma once

#include <memory>

#include <QObject>
#include <QQmlComponent>

#include "App/QuickShell/Chrome/DialogProvider.h"

class QQuickWindow;

/**
 * \class QuickDialogProvider
 * \brief Production DialogProvider for wiredpanda_quick: instantiates a fresh QML dialog
 * component per call, shows it modally over \a rootWindow, and blocks the calling (GUI)
 * thread in a nested QEventLoop until the user responds.
 *
 * \details Mirrors QDialog::exec()'s own well-established nested-event-loop pattern, so
 * ported controller code (WorkspaceManager, ICController, ...) keeps its original
 * synchronous, blocking call shape unchanged -- only the concrete provider swaps from a
 * QWidget modal to a QML one. A fresh instance is created and destroyed per call rather than
 * cached and reused: these are infrequent, always-modal interactions, so the simplicity of
 * never carrying stale state between calls outweighs the small instantiation cost.
 */
class QuickDialogProvider : public QObject, public DialogProvider
{
    Q_OBJECT

public:
    /// \param rootWindow The application's top-level window; dialogs are parented to its
    /// contentItem and centered over it. Must outlive this provider.
    explicit QuickDialogProvider(QQuickWindow *rootWindow);

    DialogButton choice(const QString &title, const QString &text,
                         const QList<DialogButton> &buttons, DialogButton defaultButton) override;
    std::optional<QString> textPrompt(const QString &title, const QString &label,
                                       const QString &initialValue) override;

signals:
    /// Emitted right after a dialog is opened (before the modal wait begins), carrying the
    /// live QML object. Lets tests drive it directly (set properties, call accept()/reject(),
    /// invoke handleButtonClicked()) instead of needing real synthetic mouse/window input --
    /// this project's Quick windows can't reliably receive that in a headless/XWayland
    /// sandbox (see project memory project_xwayland_synthetic_input_broken.md), and a native
    /// platform MessageDialog wouldn't receive it either way.
    void dialogOpened(QObject *dialog);

private:
    QQuickWindow *m_rootWindow;
    QQmlComponent m_choiceComponent;
    QQmlComponent m_textPromptComponent;
};
