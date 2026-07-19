// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinFile.h"

#include <QSaveFile>
#include <QTemporaryDir>
#include <QTextStream>

#include "App/BeWavedDolphin/DolphinFile.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Common.h"
#include "Tests/Common/TestUtils.h"

namespace {

std::unique_ptr<SignalModel> makeSignalModel(const QVector<QVector<int>> &rows, int inputRows)
{
    const int columns = rows.isEmpty() ? 0 : static_cast<int>(rows.first().size());
    auto model = std::make_unique<SignalModel>(rows.size(), columns);
    model->setInputRows(inputRows);
    for (int r = 0; r < rows.size(); ++r) {
        for (int c = 0; c < columns; ++c) {
            model->setValue(r, c, rows.at(r).at(c));
        }
    }
    return model;
}

} // namespace

void TestDolphinFile::testSaveAndLoadCSVRoundTrip()
{
    // 2 input rows + 1 output row; only the input rows are persisted/reloaded.
    auto model = makeSignalModel({{1, 0, 1, 0}, {0, 1, 0, 1}, {1, 1, 1, 1}}, 2);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/waveform.csv";

    DolphinFile::save(*model, path, 2);
    QVERIFY2(QFileInfo(path).size() > 0, "save() should have written a non-empty CSV file");

    const auto result = DolphinFile::load(path, 2);
    QCOMPARE(result.inputPorts, 2);
    QCOMPARE(result.columns, 4);
    QCOMPARE(result.values, QVector<int>({1, 0, 1, 0, 0, 1, 0, 1}));
}

void TestDolphinFile::testSaveOpenFailureThrows()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir lockedDir;
    QVERIFY(lockedDir.isValid());
    const QString path = lockedDir.path() + "/unreachable.csv";

    // QSaveFile stages its write via a sibling temp file in the same directory,
    // so stripping write+exec on the directory makes open() itself fail.
    QVERIFY(QFile::setPermissions(lockedDir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    // Sanity: confirm QSaveFile actually fails to open under this directory on this system
    // before relying on it below.
    {
        QSaveFile probe(path);
        QVERIFY(!probe.open(QIODevice::WriteOnly));
    }

    auto model = makeSignalModel({{0, 1}}, 1);
    bool threw = false;
    try {
        DolphinFile::save(*model, path, 1);
    } catch (const Pandaception &e) {
        threw = true;
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should provide a meaningful error message");
    }
    QVERIFY2(threw, "Saving into an unwritable directory should throw");

    QFile::setPermissions(lockedDir.path(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
#endif
}

void TestDolphinFile::testLoadMissingFileThrows()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/does_not_exist.dolphin";

    QVERIFY_THROWS(std::exception, DolphinFile::load(path, 4));
}

void TestDolphinFile::testLoadOpenFailureThrows()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/unreadable.csv";
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("1,1\n0\n");
    }
    QVERIFY(QFile::setPermissions(path, QFileDevice::Permissions()));

    // Sanity: confirm the file really can't be opened for reading under this permission
    // set on this system before relying on it below.
    {
        QFile probe(path);
        QVERIFY(!probe.open(QIODevice::ReadOnly));
    }

    bool threw = false;
    try {
        DolphinFile::load(path, 4);
    } catch (const Pandaception &e) {
        threw = true;
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should provide a meaningful error message");
    }
    QVERIFY2(threw, "Loading a file that cannot be opened for reading should throw");

    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif
}

void TestDolphinFile::testLoadUnsupportedFormatThrows()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/waveform.txt";
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("not a recognized format");
    }

    QVERIFY_THROWS(std::exception, DolphinFile::load(path, 4));
}

void TestDolphinFile::testParseTerminalValidInput()
{
    QString input("2,3\n1,0,1\n0,1,0\n");
    QTextStream stream(&input, QIODevice::ReadOnly);

    const auto data = DolphinFile::parseTerminal(stream, 5);
    QCOMPARE(data.inputPorts, 2);
    QCOMPARE(data.columns, 3);
    QCOMPARE(data.values, QVector<int>({1, 0, 1, 0, 1, 0}));
}

void TestDolphinFile::testParseTerminalClampsRowsToMaxInputPorts()
{
    // Header claims 5 rows but maxInputPorts clamps that to 3, so only 3 data lines
    // are consumed/stored.
    QString input("5,2\n1,1\n0,0\n1,0\n");
    QTextStream stream(&input, QIODevice::ReadOnly);

    const auto data = DolphinFile::parseTerminal(stream, 3);
    QCOMPARE(data.inputPorts, 3);
    QCOMPARE(data.columns, 2);
    QCOMPARE(data.values, QVector<int>({1, 1, 0, 0, 1, 0}));
}

void TestDolphinFile::testParseTerminalRejectsShortHeader()
{
    QString input("nocommahere\n");
    QTextStream stream(&input, QIODevice::ReadOnly);

    QVERIFY_THROWS(std::exception, DolphinFile::parseTerminal(stream, 4));
}

void TestDolphinFile::testParseTerminalRejectsNonPositiveRows()
{
    QString input("0,3\n");
    QTextStream stream(&input, QIODevice::ReadOnly);

    QVERIFY_THROWS(std::exception, DolphinFile::parseTerminal(stream, 4));
}

void TestDolphinFile::testParseTerminalRejectsColumnCountTooLow()
{
    QString input("3,0\n");
    QTextStream stream(&input, QIODevice::ReadOnly);

    QVERIFY_THROWS(std::exception, DolphinFile::parseTerminal(stream, 4));
}

void TestDolphinFile::testParseTerminalRejectsColumnCountTooHigh()
{
    QString input(QString("1,%1\n").arg(SignalModel::kMaxColumns + 1));
    QTextStream stream(&input, QIODevice::ReadOnly);

    QVERIFY_THROWS(std::exception, DolphinFile::parseTerminal(stream, 4));
}

void TestDolphinFile::testParseTerminalRejectsShortDataRow()
{
    // Header promises 3 columns per row but the only data row has 2.
    QString input("2,3\n1,0\n");
    QTextStream stream(&input, QIODevice::ReadOnly);

    QVERIFY_THROWS(std::exception, DolphinFile::parseTerminal(stream, 4));
}
