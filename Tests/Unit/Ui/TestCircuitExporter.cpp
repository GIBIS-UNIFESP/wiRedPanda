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

void TestCircuitExporter::testRenderToImageThrowsOnInvalidPath()
{
    // Path lives under a directory that doesn't exist — pixmap.save returns false
    // and pre-fix that failure was silently swallowed. The throw is the regression.
    WorkSpace workspace;
    const QString path = "/nonexistent/directory/that/does/not/exist/img.png";
    QVERIFY_EXCEPTION_THROWN(CircuitExporter::renderToImage(workspace.scene(), path), std::exception);
}

