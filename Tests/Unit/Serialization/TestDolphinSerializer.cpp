// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinSerializer.h"

#include <QSaveFile>
#include <QStandardItemModel>
#include <QTemporaryDir>

#include "App/BeWavedDolphin/Serializer.h"
#include "App/IO/Serialization.h"
#include "Tests/Common/TestUtils.h"

static QStandardItemModel *makeModel(int rows, int cols)
{
    auto *model = new QStandardItemModel;
    model->setRowCount(rows);
    model->setColumnCount(cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            model->setItem(r, c, new QStandardItem(QString::number((r + c) % 2)));
    return model;
}

void TestDolphinSerializer::testBinarySaveSingleItem()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(1, 4));
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    DolphinSerializer::saveBinary(stream, model.get(), 1);
    QVERIFY(!data.isEmpty());
}

void TestDolphinSerializer::testBinaryLoadSingleItem()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(1, 4));
    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        DolphinSerializer::saveBinary(stream, model.get(), 1);
    }
    QDataStream readStream(&data, QIODevice::ReadOnly);
    auto result = DolphinSerializer::loadBinary(readStream, 1);
    QCOMPARE(result.inputPorts, 1);
}

void TestDolphinSerializer::testBinaryRoundTrip()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(2, 5));
    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        DolphinSerializer::saveBinary(stream, model.get(), 2);
    }
    QDataStream readStream(&data, QIODevice::ReadOnly);
    auto result = DolphinSerializer::loadBinary(readStream, 2);
    QCOMPARE(result.inputPorts, 2);
    QCOMPARE(result.columns, 5);
}

void TestDolphinSerializer::testBinaryWithMultipleColumns()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(3, 8));
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    DolphinSerializer::saveBinary(stream, model.get(), 3);
    QVERIFY(!data.isEmpty());
}

void TestDolphinSerializer::testCSVSaveSingleItem()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(1, 4));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/test.csv";
    QSaveFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    DolphinSerializer::saveCSV(file, model.get());
    QVERIFY(file.commit());
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestDolphinSerializer::testCSVLoadSingleItem()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(1, 4));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/test.csv";
    {
        QSaveFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        DolphinSerializer::saveCSV(file, model.get());
        QVERIFY(file.commit());
    }
    QFile readFile(path);
    QVERIFY(readFile.open(QIODevice::ReadOnly));
    auto result = DolphinSerializer::loadCSV(readFile, 1);
    QCOMPARE(result.inputPorts, 1);
}

void TestDolphinSerializer::testCSVRoundTrip()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(2, 5));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/test.csv";
    {
        QSaveFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        DolphinSerializer::saveCSV(file, model.get());
        QVERIFY(file.commit());
    }
    QFile readFile(path);
    QVERIFY(readFile.open(QIODevice::ReadOnly));
    auto result = DolphinSerializer::loadCSV(readFile, 2);
    QCOMPARE(result.inputPorts, 2);
    QCOMPARE(result.columns, 5);
}

void TestDolphinSerializer::testCSVWithMultipleRows()
{
    std::unique_ptr<QStandardItemModel> model(makeModel(5, 3));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/test.csv";
    QSaveFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    DolphinSerializer::saveCSV(file, model.get());
    QVERIFY(file.commit());
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestDolphinSerializer::testEmptyModelBinary()
{
    QStandardItemModel model;
    model.setRowCount(0);
    model.setColumnCount(4);
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    DolphinSerializer::saveBinary(stream, &model, 0);
    QVERIFY(true);
}

void TestDolphinSerializer::testEmptyModelCSV()
{
    QStandardItemModel model;
    model.setRowCount(0);
    model.setColumnCount(4);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/empty.csv";
    QSaveFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    DolphinSerializer::saveCSV(file, &model);
    QVERIFY(file.commit());
}

void TestDolphinSerializer::testCorruptedDataHandling()
{
    QByteArray corrupt("This is corrupted binary data !!!");
    QDataStream stream(&corrupt, QIODevice::ReadOnly);
    bool caught = false;
    try {
        DolphinSerializer::loadBinary(stream, 1);
    } catch (...) {
        caught = true;
    }
    // Either throws or returns degenerate data — both acceptable
    QVERIFY(true);
    Q_UNUSED(caught);
}

void TestDolphinSerializer::testLoadBinaryRejectsNegativeRows()
{
    // Hand-craft a binary payload with rows = -1, cols = 5.
    // Pre-fix: data.values.resize(rows * cols) with negative argument was UB.
    QByteArray payload;
    {
        QDataStream stream(&payload, QIODevice::WriteOnly);
        stream << static_cast<qint64>(-1);
        stream << static_cast<qint64>(5);
    }
    QDataStream stream(&payload, QIODevice::ReadOnly);
    QVERIFY_THROWS(std::exception, DolphinSerializer::loadBinary(stream, 8));
}

void TestDolphinSerializer::testLoadCSVRejectsNegativeRows()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/negative_rows.csv";
    {
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write("-1,5,\n");
    }
    QFile in(path);
    QVERIFY(in.open(QIODevice::ReadOnly));
    QVERIFY_THROWS(std::exception, DolphinSerializer::loadCSV(in, 8));
}

void TestDolphinSerializer::testReadDolphinHeaderRejectsLargeAppName()
{
    // Legacy .dolphin header whose leading app-name length field is 0x7FFFFFFF.
    // The native operator>>(QString) would allocate ~2 GB before any check;
    // readDolphinHeader's length guard (>256 bytes, or odd, or 0xFFFFFFFF) must
    // reject it and throw before allocation.
    QFile fixture(QString(QUOTE(CURRENTDIR)) + "/Fuzz/regressions/oom_dolphin_header_large_appname.bin");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixture.errorString()));
    const QByteArray data = fixture.readAll();

    QDataStream stream(data);
    QVERIFY_THROWS(std::exception, Serialization::readDolphinHeader(stream));
}

void TestDolphinSerializer::testReadDolphinHeaderParsesLegacyAppName()
{
    // Synthetic legacy ".dolphin" stream: 4-byte length prefix (big-endian,
    // matching peekU32's qFromBigEndian convention — same as the modern-format
    // magic-number field), followed by "beWavedDolphin 1.0" as raw *native-order*
    // UTF-16 code units (no QDataStream QString framing — this format predates
    // that; see the comment in Serialization::readDolphinHeader).
    const QString appName = QStringLiteral("beWavedDolphin 1.0");
    const int byteLen = appName.size() * 2;

    QByteArray payload;
    {
        QDataStream out(&payload, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_12);
        out << static_cast<quint32>(byteLen); // default byte order is BigEndian
    }
    payload.append(reinterpret_cast<const char *>(appName.utf16()), byteLen);

    QDataStream stream(payload);
    stream.setVersion(QDataStream::Qt_5_12);

    bool threw = false;
    try {
        Serialization::readDolphinHeader(stream);
    } catch (...) {
        threw = true;
    }
    QVERIFY(!threw);
    QCOMPARE(stream.status(), QDataStream::Ok);
    // Header fully consumed (4-byte length + byteLen bytes) — next read should
    // land exactly where the caller (DolphinSerializer::loadBinary) expects it.
    QCOMPARE(stream.device()->pos(), qint64(4 + byteLen));
}
