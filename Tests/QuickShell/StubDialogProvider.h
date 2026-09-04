// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Test stub for DialogProvider (App/QuickShell/Chrome/DialogProvider.h) with
/// parameter capture. Mirrors Tests/Common/StubFileDialogProvider.h's exact shape for the
/// sibling FileDialogProvider interface -- no equivalent existed for this one yet.

#pragma once

#include "App/QuickShell/Chrome/DialogProvider.h"

/// Test stub that returns a pre-configured choice()/textPrompt() result and captures call
/// parameters, avoiding the need for a real QuickDialogProvider (which needs a live
/// QQuickWindow to parent modal Dialog items into).
class StubDialogProvider : public DialogProvider
{
public:
    DialogButton choiceResult = DialogButton::Ok;
    std::optional<QString> textPromptResult;

    struct LastChoiceCall {
        QString title;
        QString text;
        QList<DialogButton> buttons;
        DialogButton defaultButton = DialogButton::Ok;
    };
    LastChoiceCall lastChoiceCall;
    int choiceCallCount = 0;
    int textPromptCallCount = 0;

    DialogButton choice(const QString &title, const QString &text,
                        const QList<DialogButton> &buttons, DialogButton defaultButton) override
    {
        lastChoiceCall = {title, text, buttons, defaultButton};
        ++choiceCallCount;
        return choiceResult;
    }

    std::optional<QString> textPrompt(const QString &, const QString &, const QString &) override
    {
        ++textPromptCallCount;
        return textPromptResult;
    }
};

/// RAII guard that installs a StubDialogProvider and restores the previous provider on
/// destruction. Mirrors ScopedFileDialogStub exactly.
class ScopedDialogStub
{
public:
    StubDialogProvider stub;

    ScopedDialogStub()
        : m_old(Dialogs::setProvider(&stub))
    {
    }

    ~ScopedDialogStub()
    {
        Dialogs::setProvider(m_old);
    }

    ScopedDialogStub(const ScopedDialogStub &) = delete;
    ScopedDialogStub &operator=(const ScopedDialogStub &) = delete;

private:
    DialogProvider *m_old;
};
