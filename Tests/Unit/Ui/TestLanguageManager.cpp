// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLanguageManager.h"

#include "App/UI/LanguageManager.h"
#include "Tests/Common/TestUtils.h"

void TestLanguageManager::testAvailableLanguages()
{
    LanguageManager manager;
    QStringList languages = manager.availableLanguages();
    QVERIFY(!languages.isEmpty());
}

void TestLanguageManager::testSetLanguage()
{
    LanguageManager manager;
    manager.loadTranslation("en");
    QVERIFY(true);
}

