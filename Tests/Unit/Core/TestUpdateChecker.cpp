// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestUpdateChecker.h"

#include "App/Core/UpdateChecker.h"
#include "Tests/Common/TestUtils.h"

void TestUpdateChecker::testUpdateAvailable()
{
    QSKIP("UpdateChecker::onReplyFinished is private and takes a live "
          "QNetworkReply; testing the update-available path needs reply "
          "injection the class does not expose");
}

void TestUpdateChecker::testNoUpdate()
{
    QSKIP("UpdateChecker::onReplyFinished is private and takes a live "
          "QNetworkReply; testing the no-update path needs reply injection "
          "the class does not expose");
}

void TestUpdateChecker::testNetworkError()
{
    QSKIP("UpdateChecker::onReplyFinished is private and takes a live "
          "QNetworkReply; testing the network-error path needs reply "
          "injection the class does not expose");
}
