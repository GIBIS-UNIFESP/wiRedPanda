// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief FileDialogs::provider()/setProvider() global-state storage.
 *
 * \details Split out of App/UI/FileDialogProvider.cpp (which needs QFileDialog, Widgets-only,
 * to implement RealFileDialogProvider) so this storage itself -- needed by any
 * FileDialogProvider consumer, including the Quick-side QuickDialogProvider -- doesn't pull
 * in a Widgets dependency. No automatic RealFileDialogProvider default: unlike the original
 * pre-split behavior, a provider must be explicitly registered via setProvider() before
 * provider() is called, mirroring App/QuickShell/Chrome/DialogProvider.h's Dialogs::provider()
 * contract exactly. Each concrete UI (Widgets' MainWindow.cpp, Quick's Main.cpp) is
 * responsible for registering its own real provider at startup.
 */

#include "App/UI/FileDialogProvider.h"

namespace {

FileDialogProvider *s_provider = nullptr;

} // namespace

FileDialogProvider *FileDialogs::provider()
{
    Q_ASSERT(s_provider);
    return s_provider;
}

FileDialogProvider *FileDialogs::setProvider(FileDialogProvider *newProvider)
{
    auto *old = s_provider;
    s_provider = newProvider;
    return old;
}
