// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Test stub for MainWindowHost with parameter capture, shared by any controller
/// test (WorkspaceManager, ExportController, ICController, ...) that needs one.

#pragma once

#include "App/UI/MainWindowHost.h"

/// Test stub that returns pre-configured values and captures call parameters. Fields are
/// public and directly settable/readable -- there is no behavior to hide, just plumbing.
class StubMainWindowHost : public MainWindowHost
{
public:
    // --- Pre-configured return values (set before calling code-under-test) ---

    WorkSpace *currentTabValue = nullptr;
    QFileInfo currentFileValue;
    QDir currentDirValue;
    QFileInfo icListFileValue;
    ElementPalette *paletteValue = nullptr;
    QWidget *widgetValue = nullptr;
    DolphinHost *dolphinHostValue = nullptr;

    // --- Captured calls (read after calling code-under-test) ---

    struct StatusMessage {
        QString message;
        int timeout = 0;
    };
    QList<StatusMessage> statusMessages;
    int requestSaveCallCount = 0;
    int setICButtonsVisibleCallCount = 0;
    bool lastICButtonsVisible = true;
    int refreshICButtonsEnabledCallCount = 0;

    WorkSpace *currentTab() const override { return currentTabValue; }
    QFileInfo currentFile() const override { return currentFileValue; }
    QDir currentDir() const override { return currentDirValue; }
    QFileInfo icListFile() const override { return icListFileValue; }
    ElementPalette *palette() const override { return paletteValue; }
    QWidget *widget() override { return widgetValue; }
    DolphinHost *dolphinHost() override { return dolphinHostValue; }

    void showStatusMessage(const QString &message, int timeout) override
    {
        statusMessages.append({message, timeout});
    }

    void requestSave() override { ++requestSaveCallCount; }

    void setICButtonsVisible(bool visible) override
    {
        ++setICButtonsVisibleCallCount;
        lastICButtonsVisible = visible;
    }

    void refreshICButtonsEnabled() override { ++refreshICButtonsEnabledCallCount; }
};
