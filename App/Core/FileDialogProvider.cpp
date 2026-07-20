// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/FileDialogProvider.h"

#include <QtGlobal>

static FileDialogProvider *s_defaultProvider = nullptr;
static FileDialogProvider *s_provider = nullptr;

FileDialogProvider *FileDialogs::provider()
{
    Q_ASSERT_X(s_provider, "FileDialogs::provider",
               "setDefaultProvider() must be called once at startup before first use");
    return s_provider;
}

void FileDialogs::setDefaultProvider(FileDialogProvider *provider)
{
    s_defaultProvider = provider;
    s_provider = provider;
}

FileDialogProvider *FileDialogs::setProvider(FileDialogProvider *newProvider)
{
    auto *old = s_provider;
    s_provider = newProvider ? newProvider : s_defaultProvider;
    return old;
}
