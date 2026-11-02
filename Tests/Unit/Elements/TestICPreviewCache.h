// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestICPreviewCache : public QObject
{
    Q_OBJECT

private slots:

    void init();

    void testLoadingRendersNoPreview();
    void testFirstAskRendersAndKeeps();
    void testFileInstancesShareOnePreview();
    void testBlobInstancesShareOnePreviewPerDirectory();
    void testReloadAsksForItsOwn();
    void testNullPreviewIsKept();
    void testCircuitThatNoLongerBuildsHasANullPreviewKept();
    void testLeastRecentlyUsedPreviewGoes();
    void testThemeChangeDropsEveryPreview();
};
