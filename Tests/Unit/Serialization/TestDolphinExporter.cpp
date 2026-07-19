// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinExporter.h"

#include <QFile>
#include <QPainter>
#include <QPrinter>
#include <QTemporaryDir>

#include "App/BeWavedDolphin/DolphinExporter.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "Tests/Common/TestUtils.h"

void TestDolphinExporter::testExportToPdfThrowsWhenPrinterCannotOpen()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir lockedDir;
    QVERIFY(lockedDir.isValid());
    const QString pdfPath = lockedDir.path() + "/out.pdf";

    QVERIFY(QFile::setPermissions(lockedDir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    // Sanity: confirm QPrinter really can't open the output file under this directory on
    // this system before relying on it below.
    {
        QPrinter probe(QPrinter::HighResolution);
        probe.setOutputFileName(pdfPath);
        QPainter painter;
        QVERIFY(!painter.begin(&probe));
    }

    SignalModel model(1, 2);
    model.setValue(0, 0, 1);

    QVERIFY_THROWS(std::exception, DolphinExporter::exportToPdf(&model, PlotType::Line, pdfPath));

    QFile::setPermissions(lockedDir.path(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
#endif
}
