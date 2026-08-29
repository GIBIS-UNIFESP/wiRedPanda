// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestInputElements : public QObject
{
    Q_OBJECT

private slots:
    /// A sweep resets every element for reproducibility, and InputRotary must take part.
    /// m_currentPort is not an output value, and updateOutputs() republishes it onto the outputs
    /// every tick, so a reset that misses it is overwritten before anything reads it. A default
    /// all-zero waveform table cannot correct that either -- setWaveformValue() is deliberately a
    /// no-op for a low cell -- which would leave every column running at whatever position the
    /// user last left on the canvas.
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

    // Path resolution (mechanics tested directly in Tests/Unit/Core/TestExternalFilePath.cpp;
    // these confirm InputSwitch/appearance handling itself is wired to it correctly)
    void testAppearanceWithSameOsAbsolutePath();
    void testAppearanceWithNonExistentFileFallback();
    void testAppearanceReloadsAfterFileModified();
    void testLoadResolvesAppearanceBareFilenameAgainstContextDir();
    void testLoadResolvesAppearanceForeignPathViaBareFilenameFallback();
};
