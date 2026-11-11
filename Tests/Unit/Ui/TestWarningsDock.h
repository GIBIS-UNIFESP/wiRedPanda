// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestWarningsDock : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void testRowsAndCount();
    void testActivation();
    void testRefreshAfterCircuitHasChangedAndTabContentLoaded();
    void testToggleVisibility();
    void testDropElementUpdatesWarningsPanel();
    void testWithRealSettings();
    void testLoadSavedFile();
    void testElementHighlightRendering();
    void testLoadFileWithUnconnectedElements();
    void testEveryComponentTypeTriggersWarningsWhenInputsOpened();
};
