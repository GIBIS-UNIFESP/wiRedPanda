// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// General behavioral coverage for MCP/Server/Handlers/ICHandler.cpp: create_ic/
/// instantiate_ic/list_ics/embed_ic/extract_ic's validation chains and real success
/// paths. Path-traversal/security hardening lives in TestICHandlerSecurity instead.
class TestICHandler : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();

    void testHandleCommandRejectsUnknownCommand();

    void testHandleCreateIcRejectsMissingParams();
    void testHandleCreateIcRejectsNoScene();
    void testHandleCreateIcRejectsEmptyName();
    void testHandleCreateIcRejectsNameWithPathSeparator();
    void testHandleCreateIcCreatesRealFile();
    void testHandleCreateIcRejectsExistingFile();
    void testHandleCreateIcRejectsEmptyCircuit();
    void testHandleCreateIcFailsOnUnwritableTarget();

    void testHandleInstantiateIcRejectsMissingParams();
    void testHandleInstantiateIcRejectsNoScene();
    void testHandleInstantiateIcRejectsEmptyName();
    void testHandleInstantiateIcRejectsAbsolutePathWithDotDot();
    void testHandleInstantiateIcRejectsRelativeNameWithPathSeparator();
    void testHandleInstantiateIcRejectsMissingFile();
    void testHandleInstantiateIcInstantiatesRealFileBackedIc();
    void testHandleInstantiateIcInstantiatesViaAbsolutePath();
    void testHandleInstantiateIcInlineRejectsUnreadableFile();
    void testHandleInstantiateIcInlineDefaultsBlobNameToBaseName();
    void testHandleInstantiateIcInlineRejectsBlobNameCollision();
    void testHandleInstantiateIcInlineCreatesRealEmbeddedIc();

    void testHandleListIcsReturnsRealDefinitions();
    void testHandleListIcsSkipsUnreadableFile();
    void testHandleListIcsMarksCorruptBodyAsInvalidDefinition();
    void testHandleListIcsSkipsCorruptHeaderViaOuterCatch();

    void testHandleEmbedIcRejectsMissingParams();
    void testHandleEmbedIcRejectsNoScene();
    void testHandleEmbedIcRejectsNonPositiveElementId();
    void testHandleEmbedIcRejectsUnknownElement();
    void testHandleEmbedIcRejectsNonIcElement();
    void testHandleEmbedIcRejectsAlreadyEmbedded();
    void testHandleEmbedIcRejectsNoReferencedFile();
    void testHandleEmbedIcRejectsProjectNotSaved();
    void testHandleEmbedIcRejectsUnreadableFile();
    void testHandleEmbedIcRejectsBlobNameCollision();
    void testHandleEmbedIcEmbedsRealFileDefaultingBlobName();

    void testHandleExtractIcRejectsMissingParams();
    void testHandleExtractIcRejectsNoScene();
    void testHandleExtractIcRejectsProjectNotSaved();
    void testHandleExtractIcRejectsEmptyBlobName();
    void testHandleExtractIcRejectsUnknownBlob();
    void testHandleExtractIcAppendsPandaExtension();
    void testHandleExtractIcRejectsExistingFileWithoutOverwrite();
    void testHandleExtractIcOverwritesExistingFileWhenRequested();
    void testHandleExtractIcExtractsRealFileWithDefaultName();
};
