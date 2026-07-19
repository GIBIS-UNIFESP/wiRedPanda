// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestGraphicElementSerializer : public QObject
{
    Q_OBJECT

private slots:
    // readPortList() truncated-stream guards
    void testReadPortListCountReadFailureThrows();
    void testReadPortListEntryReadFailureThrows();

    // removePortFromMap()
    void testRemovePortFromMapErasesMatchingEntries();

    // loadOldFormat()'s pre-4.0.1 unused-priority field
    void testLoadOldFormatDrainsUnusedPriorityField();

    // loadNewFormat()'s truncated-properties-map guard and appearance-index guard
    void testLoadNewFormatPropertiesStreamErrorThrows();
    void testLoadNewFormatAppearanceIndexOutOfRangeThrows();

    // loadInputPorts()/loadOutputPorts()'s implausible-count guards (old format)
    void testLoadInputPortsImplausibleCountThrows();
    void testLoadOutputPortsImplausibleCountThrows();

    // loadPixmapAppearanceNames()'s implausible-count and index-range guards, and the
    // successful (non-IC) apply path
    void testLoadPixmapAppearanceNamesImplausibleCountThrows();
    void testLoadPixmapAppearanceNamesIndexOutOfRangeThrows();
    void testLoadPixmapAppearanceNamesAppliesResolvedPath();

    // loadInputPort()/loadOutputPort()'s IC-only rename-existing-port branch
    void testLoadInputOutputPortRenamesExistingIcPorts();
};
