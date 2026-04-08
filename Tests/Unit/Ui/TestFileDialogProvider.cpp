// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestFileDialogProvider.h"

#include "App/UI/FileDialogProvider.h"
#include "Tests/Common/TestUtils.h"

void TestFileDialogProviderUnit::testFileDialogCreation()
{
    // FileDialogProvider is abstract — verify global accessor works
    QVERIFY(FileDialogs::provider() != nullptr);
}

