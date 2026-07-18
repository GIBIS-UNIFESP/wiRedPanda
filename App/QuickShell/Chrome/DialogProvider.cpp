// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/DialogProvider.h"

namespace {

// Unlike FileDialogs' RealFileDialogProvider, the production QuickDialogProvider needs a
// QQuickWindow at construction time, so there's no parameterless default to fall back on
// here -- Main.cpp must construct one and call Dialogs::setProvider() before any controller
// code that could call Dialogs::provider() runs.
DialogProvider *s_provider = nullptr;

} // namespace

DialogProvider *Dialogs::provider()
{
    Q_ASSERT(s_provider);
    return s_provider;
}

DialogProvider *Dialogs::setProvider(DialogProvider *newProvider)
{
    auto *old = s_provider;
    s_provider = newProvider;
    return old;
}
