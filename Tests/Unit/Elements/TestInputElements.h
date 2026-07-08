// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestInputElements : public QObject
{
    Q_OBJECT

private slots:
    // InputSwitch Tests
    void testInputSwitchConstructor();
    void testInputSwitchInitialState();
    void testInputSwitchSetOnToggle();
    void testInputSwitchSetOffToggle();
    void testInputSwitchIsOn();
    void testInputSwitchPortStatus();
    void testInputSwitchSaveState();
    void testInputSwitchLoadOldVersion();
    void testInputSwitchLoadNewVersion();
    void testInputSwitchLockingMechanism();
    void testInputSwitchAppearanceChange();

    // InputButton Tests
    void testInputButtonConstructor();
    void testInputButtonInitialState();
    void testInputButtonSetOn();
    void testInputButtonSetOff();
    void testInputButtonIsOn();
    void testInputButtonPortStatus();
    void testInputButtonSaveState();
    void testInputButtonLoadOldVersion();
    void testInputButtonLoadNewVersion();
    void testInputButtonLockingMechanism();
    void testInputButtonAppearanceChange();

    // Basic InputButton test (migrated from testelements)
    void testInputButton();

    // Basic InputSwitch test (migrated from testelements)
    void testInputSwitch();

    // Path resolution (mechanics tested directly in Tests/Unit/Core/TestExternalFilePath.cpp;
    // these confirm InputSwitch/appearance handling itself is wired to it correctly)
    void testAppearanceWithSameOsAbsolutePath();
    void testAppearanceWithNonExistentFileFallback();
    void testAppearanceReloadsAfterFileModified();
    void testLoadResolvesAppearanceBareFilenameAgainstContextDir();
    void testLoadResolvesAppearanceForeignPathViaBareFilenameFallback();
};
