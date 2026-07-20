// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestFileHandler : public QObject
{
    Q_OBJECT

private slots:
    void testHandleLoadCircuitRejectsMissingFilename();
    void testHandleLoadCircuitRejectsEmptyFilename();
    void testHandleLoadCircuitRejectsNoMainWindow();
    void testHandleLoadCircuitLoadsRealFile();

    void testHandleSaveCircuitRejectsMissingFilename();
    void testHandleSaveCircuitRejectsEmptyFilename();
    void testHandleSaveCircuitRejectsWrongExtension();
    void testHandleSaveCircuitRejectsNoMainWindow();
    void testHandleSaveCircuitSavesRealFile();

    void testHandleNewCircuitRejectsNoMainWindow();
    void testHandleNewCircuitCreatesRealTab();

    void testHandleCloseCircuitRejectsNoMainWindow();
    void testHandleCloseCircuitRejectsNoTabs();
    void testHandleCloseCircuitClosesRealTab();

    void testHandleGetTabCountRejectsNoMainWindow();
    void testHandleGetTabCountReturnsRealCount();

    void testHandleExportImageRejectsMissingParams();
    void testHandleExportImageRejectsEmptyFilenameOrFormat();
    void testHandleExportImageRejectsNoScene();
    void testHandleExportImageRejectsUnsupportedFormat();
    void testHandleExportImageExportsRealPng();
    void testHandleExportImageExportsRealSvg();
    void testHandleExportImageExportsRealPdf();
    void testHandleExportImageRejectsSvgOpenFailure();
    void testHandleExportImageRejectsSvgWriteFailure();
    void testHandleExportImageRejectsPdfOpenFailure();
    void testHandleExportImageRejectsPngSaveFailure();

    void testHandleExportArduinoRejectsMissingFilename();
    void testHandleExportArduinoRejectsEmptyFilename();
    void testHandleExportArduinoRejectsNoMainWindow();
    void testHandleExportArduinoExportsRealFile();

    void testHandleExportSystemVerilogRejectsMissingFilename();
    void testHandleExportSystemVerilogRejectsEmptyFilename();
    void testHandleExportSystemVerilogRejectsNoMainWindow();
    void testHandleExportSystemVerilogExportsRealFile();

    void testHandleCommandRejectsUnknownCommand();
};
