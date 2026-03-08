// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>

#include <QObject>

class TestClock : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testConstructorInitialization();

    // Frequency management tests
    void testDefaultFrequency();
    void testSetFrequency();
    void testSetFrequencyZero();
    void testSetFrequencyNegative();
    void testFrequencyPersistence();
    void testFrequencyHighValues();

    // Delay management tests
    void testDefaultDelay();
    void testSetDelay();
    void testSetDelayNegative();
    void testDelayPersistence();

    // State management tests
    void testInitialState();
    void testSetOn();
    void testSetOff();
    void testIsOn();
    void testPortStatus();

    // Timing tests
    void testResetClock();
    void testUpdateClock();

    // Serialization tests
    void testSaveFrequencyDelay();
    void testLoadVersionOld();
    void testLoadVersionNew();
    void testLoadVersionVeryOld();

    // Generic properties tests
    void testGenericProperties();
};

