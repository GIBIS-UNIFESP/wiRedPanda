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
    void testInputSwitchSkinChange();

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
    void testInputButtonSkinChange();

    // Basic InputButton test (migrated from testelements)
    void testInputButton();

    // Basic InputSwitch test (migrated from testelements)
    void testInputSwitch();
};

