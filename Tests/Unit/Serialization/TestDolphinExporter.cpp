// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinExporter.h"

#include <QFile>
#include <QPainter>
#include <QPrinter>
#include <QTemporaryDir>

#include "App/BeWavedDolphin/DolphinExporter.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Enums.h"
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

void TestDolphinExporter::testTruthTableTextWritesOneCharacterPerThreeStateCell()
{
    // The format has no separator between cells, so a row is only parseable while every value is
    // one character wide. Status is four-state (Unknown = -1, Error = 2), so writing the raw int
    // would widen a cell to two characters and shift every column after it.
    SignalModel model(2, 4);
    model.setInputRows(1);
    model.setVerticalHeaderLabels({QStringLiteral("in"), QStringLiteral("out")});

    model.setValue(0, 0, static_cast<int>(Status::Unknown));
    model.setValue(0, 1, 1);
    model.setValue(0, 2, 0);
    model.setValue(0, 3, static_cast<int>(Status::Error));
    for (int col = 0; col < 4; ++col) {
        model.setValue(1, col, col % 2);
    }

    QString text;
    QTextStream out(&text);
    DolphinExporter::writeTruthTableText(out, &model, 1);
    out.flush();

    const auto lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    bool sawRow = false;
    for (const QString &line : lines) {
        const int sep = static_cast<int>(line.indexOf(QStringLiteral(" : ")));
        if (sep < 0) {
            continue;
        }
        sawRow = true;
        QCOMPARE(sep, 4); // exactly one character per column, for all four columns
        QVERIFY2(!line.left(sep).contains(QLatin1Char('-')),
                 qPrintable(QStringLiteral("a raw negative Status leaked into the export: %1").arg(line)));
    }
    QVERIFY2(sawRow, "precondition: the export must contain at least one labelled data row");
}

void TestDolphinExporter::testCsvTextKeepsFourStateIntsOnOutputRows()
{
    // The two exporters in this file encode a non-definite cell differently, and that is
    // deliberate rather than an oversight: writeTruthTableText() has no separator between
    // cells, so it must spend exactly one character and writes x/E; csvText() is
    // comma-separated and keeps the raw four-state Status int, which is the encoding a
    // consuming script wants and the one create_waveform declares over MCP. Pin both, so a
    // future "make these consistent" pass has to confront the reason.
    SignalModel model(2, 2);
    model.setInputRows(1);
    model.setValue(0, 0, 1);
    model.setValue(0, 1, 0);
    model.setValue(1, 0, static_cast<int>(Status::Unknown));
    model.setValue(1, 1, static_cast<int>(Status::Error));

    const QString csv = DolphinExporter::csvText(&model);
    const auto lines = csv.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QCOMPARE(lines.size(), 3);                       // header + one line per row
    QCOMPARE(lines.at(0), QStringLiteral("2,2,"));
    QCOMPARE(lines.at(1), QStringLiteral("1,0,"));   // input row: two-state (SignalModel clamps writes)
    QCOMPARE(lines.at(2), QStringLiteral("-1,2,"));  // output row: raw four-state

    QString truthTable;
    QTextStream out(&truthTable);
    DolphinExporter::writeTruthTableText(out, &model, 1);
    out.flush();
    QVERIFY2(truthTable.contains(QStringLiteral("xE")),
             qPrintable(QStringLiteral("the truth-table sibling must still spend one character per cell: %1").arg(truthTable)));
}
