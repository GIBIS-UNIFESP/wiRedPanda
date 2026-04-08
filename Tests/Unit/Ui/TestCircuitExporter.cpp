// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestCircuitExporter.h"

#include <QTemporaryDir>

#include "App/Scene/Workspace.h"
#include "App/UI/CircuitExporter.h"
#include "Tests/Common/TestUtils.h"

void TestCircuitExporter::testExporterCreation()
{
    // CircuitExporter is a namespace — test that renderToImage doesn't crash on empty scene
    WorkSpace workspace;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/test.png";
    CircuitExporter::renderToImage(workspace.scene(), path);
    QVERIFY(true);
}

