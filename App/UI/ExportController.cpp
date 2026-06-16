// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ExportController.h"

#include <QDesktopServices>
#include <QDir>
#include <QLoggingCategory>
#include <QUrl>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/CircuitExporter.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/MainWindowHost.h"

ExportController::ExportController(MainWindowHost &host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
}

void ExportController::exportToArduino(QString fileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    auto elements = tab->scene()->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The .panda file is empty.");
    }

    // Pause the simulation while generating code to avoid data races between
    // the simulation timer and the code generator reading element state.
    SimulationBlocker simulationBlocker(tab->simulation());

    if (!fileName.endsWith(".ino")) {
        fileName.append(".ino");
    }

    ArduinoCodeGen arduino(QDir::home().absoluteFilePath(fileName), elements);
    arduino.generate();
    m_host.showStatusMessage(tr("Arduino code successfully generated."), 4000);

    qCDebug(zero) << "Arduino code successfully generated.";
}

void ExportController::exportToSystemVerilog(QString fileName)
{
    auto *tab = m_host.currentTab();
    if (!tab) {
        return;
    }

    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    auto elements = tab->scene()->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The .panda file is empty.");
    }

    SimulationBlocker simulationBlocker(tab->simulation());

    if (!fileName.endsWith(".sv")) {
        fileName.append(".sv");
    }

    SystemVerilogCodeGen verilog(QDir::home().absoluteFilePath(fileName), elements);
    verilog.generate();
    m_host.showStatusMessage(tr("SystemVerilog code successfully generated."), 4000);

    qCDebug(zero) << "SystemVerilog code successfully generated.";
}

void ExportController::exportToWaveFormFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    // Headless export: a stack instance is never shown/closed, so it cleans up at scope end
    // (its WA_DeleteOnClose only fires on a close event) — and RAII-frees even if this throws.
    BewavedDolphin bewavedDolphin(m_host.currentTab()->scene(), false, m_host.dolphinHost());
    bewavedDolphin.createWaveform(fileName);
    bewavedDolphin.print();
}

void ExportController::exportToWaveFormTerminal()
{
    BewavedDolphin bewavedDolphin(m_host.currentTab()->scene(), false, m_host.dolphinHost());
    bewavedDolphin.createWaveform();
    bewavedDolphin.print();
}

void ExportController::exportArduinoDialog()
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

        const QString fileName = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Generate Arduino Code"), path, tr("Arduino file (*.ino)")).fileName;

        if (!fileName.isEmpty()) {
            exportToArduino(fileName);
        }
    });
}

void ExportController::exportSystemVerilogDialog()
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

        const QString fileName = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Generate SystemVerilog Code"), path, tr("SystemVerilog file (*.sv)")).fileName;

        if (!fileName.isEmpty()) {
            exportToSystemVerilog(fileName);
        }
    });
}

void ExportController::exportPdfDialog()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Export to PDF"));
        auto *tab = m_host.currentTab();
        if (!tab) {
            return;
        }

        // De-select elements so their selection handles don't appear in the export.
        tab->scene()->clearSelection();

        const QString path    = m_host.currentFile().exists() ? m_host.currentFile().absolutePath() : QString();
        QString pdfFile = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Export to PDF"), path, tr("PDF files (*.pdf)")).fileName;

        if (pdfFile.isEmpty()) {
            return;
        }

        if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
            pdfFile.append(".pdf");
        }

        CircuitExporter::renderToPdf(tab->scene(), pdfFile);
        m_host.showStatusMessage(tr("Exported file successfully."), 4000);
        QDesktopServices::openUrl(QUrl::fromLocalFile(pdfFile));
    });
}

void ExportController::exportImageDialog()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("export", QStringLiteral("Export to image"));
        auto *tab = m_host.currentTab();
        if (!tab) {
            return;
        }

        // De-select elements so their selection handles don't appear in the export.
        tab->scene()->clearSelection();

        const QString path    = m_host.currentFile().exists() ? m_host.currentFile().absolutePath() : QString();
        QString pngFile = FileDialogs::provider()->getSaveFileName(m_host.widget(), tr("Export to Image"), path, tr("PNG files (*.png)")).fileName;

        if (pngFile.isEmpty()) {
            return;
        }

        if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
            pngFile.append(".png");
        }

        CircuitExporter::renderToImage(tab->scene(), pngFile);
        m_host.showStatusMessage(tr("Exported file successfully."), 4000);
        QDesktopServices::openUrl(QUrl::fromLocalFile(pngFile));
    });
}
