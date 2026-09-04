// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// CanvasItem owns contextDir()/setContextDir() and addItem() itself, so these tests drive it
/// directly with no separate workspace object needed. testInputSwitchMousePressWhileLockedDoesNotToggle/
/// testInputButtonMousePressReleaseWhileLockedDoesNotToggle drive input via a real `QMouseEvent`
/// sent through `QCoreApplication::sendEvent()`, adding elements via `CanvasAddItemsCommand`
/// (not a bare `addItem()`, which doesn't populate the spatial index the hit test reads).
class TestInputElements : public QObject
{
    Q_OBJECT

private slots:
    void testRotarySelectionIsResetAndRestoredWithSimState();

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
    void testInputSwitchMousePressWhileLockedDoesNotToggle();
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
    void testInputButtonMousePressReleaseWhileLockedDoesNotToggle();
    void testInputButtonAppearanceChange();

    // Basic InputButton test (migrated from testelements)
    void testInputButton();

    // Basic InputSwitch test (migrated from testelements)
    void testInputSwitch();

    // Path resolution (mechanics tested directly in TestExternalFilePath.cpp; these confirm
    // InputSwitch/appearance handling itself is wired to it correctly)
    void testAppearanceWithSameOsAbsolutePath();
    void testAppearanceWithNonExistentFileFallback();
    void testAppearanceReloadsAfterFileModified();
    void testLoadResolvesAppearanceBareFilenameAgainstContextDir();
    void testLoadResolvesAppearanceForeignPathViaBareFilenameFallback();
};
