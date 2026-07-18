// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickExportController.h"

#include <QDir>

#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickMainWindowHost.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/FileDialogProvider.h"

QuickExportController::QuickExportController(QuickMainWindowHost &host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
}

void QuickExportController::exportToArduino(QString fileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    auto elements = tab->canvas()->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The .panda file is empty.");
    }

    SimulationBlocker simulationBlocker(tab->canvas()->simulation());

    if (!fileName.endsWith(".ino")) {
        fileName.append(".ino");
    }

    ArduinoCodeGen arduino(QDir::home().absoluteFilePath(fileName), elements);
    arduino.generate();
    m_host.showStatusMessage(tr("Arduino code successfully generated."), 4000);
}

void QuickExportController::exportToSystemVerilog(QString fileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    auto elements = tab->canvas()->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The .panda file is empty.");
    }

    SimulationBlocker simulationBlocker(tab->canvas()->simulation());

    if (!fileName.endsWith(".sv")) {
        fileName.append(".sv");
    }

    SystemVerilogCodeGen verilog(QDir::home().absoluteFilePath(fileName), elements);
    verilog.generate();
    m_host.showStatusMessage(tr("SystemVerilog code successfully generated."), 4000);
}

void QuickExportController::exportArduinoDialog()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Export to Arduino"));
        if (!m_host.currentTab()) {
            return;
        }

        QString path;
        if (m_host.currentFile().exists()) {
            path = m_host.currentFile().absolutePath();
        }

        const QString fileName = FileDialogs::provider()->getSaveFileName(nullptr, tr("Generate Arduino Code"), path, tr("Arduino file") + " (*.ino)").fileName;

        if (!fileName.isEmpty()) {
            exportToArduino(fileName);
        }
    });
}

void QuickExportController::exportSystemVerilogDialog()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Export to SystemVerilog"));
        if (!m_host.currentTab()) {
            return;
        }

        QString path;
        if (m_host.currentFile().exists()) {
            path = m_host.currentFile().absolutePath();
        }

        const QString fileName = FileDialogs::provider()->getSaveFileName(nullptr, tr("Generate SystemVerilog Code"), path, tr("SystemVerilog file") + " (*.sv)").fileName;

        if (!fileName.isEmpty()) {
            exportToSystemVerilog(fileName);
        }
    });
}
