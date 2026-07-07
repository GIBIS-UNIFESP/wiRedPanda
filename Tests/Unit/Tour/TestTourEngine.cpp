// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Tour/TestTourEngine.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Core/Settings.h"
#include "App/Tour/TourEngine.h"

namespace {

const char *const kTourFixtureId = "test-retranslate-tour";

const char *const kTourFixtureJson = R"({
    "id": "test-retranslate-tour",
    "title": "Test Tour",
    "steps": [
        { "key": "step0", "title": "Title 0", "body": "Body 0" },
        { "key": "step1", "title": "Title 1", "body": "Body 1" }
    ]
})";

QString writeTourFixture(const QTemporaryDir &dir)
{
    const QString path = dir.filePath("tour.json");
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(kTourFixtureJson);
    return path;
}

} // namespace

void TestTourEngine::testRetranslateBeforeLoadIsNoOp()
{
    TourEngine engine;
    QSignalSpy retranslatedSpy(&engine, &TourEngine::retranslated);

    engine.retranslate();

    QVERIFY(!engine.isActive());
    QCOMPARE(retranslatedSpy.count(), 0);
}

void TestTourEngine::testRetranslateWhileInactiveIsNoOp()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    QVERIFY(!engine.isActive()); // loadFromResource() alone never activates the engine

    QSignalSpy retranslatedSpy(&engine, &TourEngine::retranslated);
    engine.retranslate();

    QVERIFY(!engine.isActive());
    QCOMPARE(retranslatedSpy.count(), 0);
}

void TestTourEngine::testRetranslatePreservesProgressAndData()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("en");
    Settings::setTourProgress(QLatin1String(kTourFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();

    QCOMPARE(engine.currentStep(), 1);
    QVERIFY(engine.isActive());

    engine.retranslate();

    QCOMPARE(engine.currentStep(), 1);
    QVERIFY(engine.isActive());
    QCOMPARE(engine.tourTitle(), QStringLiteral("Test Tour"));
    QCOMPARE(engine.currentStepData().title, QStringLiteral("Title 1"));
    QCOMPARE(engine.currentStepData().body, QStringLiteral("Body 1"));

    Settings::setLanguage(originalLang);
}

void TestTourEngine::testRetranslateEmitsRetranslatedOnly()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("en");
    Settings::setTourProgress(QLatin1String(kTourFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    QSignalSpy retranslatedSpy(&engine, &TourEngine::retranslated);
    QSignalSpy stepChangedSpy(&engine, &TourEngine::stepChanged);

    engine.retranslate();

    QCOMPARE(retranslatedSpy.count(), 1);
    QCOMPARE(stepChangedSpy.count(), 0);

    Settings::setLanguage(originalLang);
}
