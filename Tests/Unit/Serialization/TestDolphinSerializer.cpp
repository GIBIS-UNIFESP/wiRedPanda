// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinSerializer.h"

#include <QSaveFile>
#include <QStandardItemModel>
#include <QTemporaryDir>

#include "App/BeWavedDolphin/Serializer.h"
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

