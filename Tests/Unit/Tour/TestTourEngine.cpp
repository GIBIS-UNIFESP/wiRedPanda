// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Tour/TestTourEngine.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Core/Settings.h"
#include "App/Tour/TourEngine.h"
#include "App/Tour/TourOverlay.h"

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
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    file.write(kTourFixtureJson);
    return path;
}

QString writeJsonFile(const QTemporaryDir &dir, const QString &name, const char *contents)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    file.write(contents);
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

void TestTourEngine::testOverlayFontScalesWithApplicationFont()
{
    const QFont originalFont = QApplication::font();

    QFont smallFont = originalFont;
    smallFont.setPointSize(8);
    QApplication::setFont(smallFont);
    const int smallPx = TourOverlay::scaledFontPx(13);

    QFont largeFont = originalFont;
    largeFont.setPointSize(24);
    QApplication::setFont(largeFont);
    const int largePx = TourOverlay::scaledFontPx(13);

    QApplication::setFont(originalFont);

    QVERIFY2(largePx > smallPx, "scaledFontPx must grow with the application's font size, "
                                "so the tour overlay's text respects an OS/Qt font-scale setting");
}

void TestTourEngine::testStartWithoutLoadIsNoOp()
{
    TourEngine engine;
    QSignalSpy stepChangedSpy(&engine, &TourEngine::stepChanged);

    engine.start();

    QVERIFY(!engine.isActive());
    QCOMPARE(stepChangedSpy.count(), 0);
}

void TestTourEngine::testAdvanceStepWhileInactiveIsNoOp()
{
    TourEngine engine;
    QSignalSpy stepChangedSpy(&engine, &TourEngine::stepChanged);
    QSignalSpy completedSpy(&engine, &TourEngine::tourCompleted);

    engine.advanceStep();

    QCOMPARE(engine.currentStep(), 0);
    QCOMPARE(stepChangedSpy.count(), 0);
    QCOMPARE(completedSpy.count(), 0);
}

void TestTourEngine::testStopWhileActiveEmitsAndDeactivates()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    QVERIFY(engine.isActive());

    QSignalSpy stoppedSpy(&engine, &TourEngine::tourStopped);
    engine.stop();

    QVERIFY(!engine.isActive());
    QCOMPARE(stoppedSpy.count(), 1);
}

void TestTourEngine::testStopWhileInactiveIsNoOp()
{
    TourEngine engine;
    QSignalSpy stoppedSpy(&engine, &TourEngine::tourStopped);

    engine.stop();

    QCOMPARE(stoppedSpy.count(), 0);
}

void TestTourEngine::testGoToPreviousStepMovesBackAndPersists()
{
    Settings::setTourProgress(QLatin1String(kTourFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();
    QCOMPARE(engine.currentStep(), 1);

    QSignalSpy stepChangedSpy(&engine, &TourEngine::stepChanged);
    engine.goToPreviousStep();

    QCOMPARE(engine.currentStep(), 0);
    QCOMPARE(stepChangedSpy.count(), 1);
    QCOMPARE(Settings::tourProgress(QLatin1String(kTourFixtureId)), 0);
}

void TestTourEngine::testGoToPreviousStepAtFirstStepIsNoOp()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    QSignalSpy stepChangedSpy(&engine, &TourEngine::stepChanged);
    engine.goToPreviousStep();

    QCOMPARE(engine.currentStep(), 0);
    QCOMPARE(stepChangedSpy.count(), 0);
}

void TestTourEngine::testGoToPreviousStepWhileInactiveIsNoOp()
{
    TourEngine engine;
    QSignalSpy stepChangedSpy(&engine, &TourEngine::stepChanged);

    engine.goToPreviousStep();

    QCOMPARE(stepChangedSpy.count(), 0);
}

void TestTourEngine::testAdvanceStepReachesEndMarksCompletedAndEmits()
{
    // Reaching the last step's advance() call must mark the tour completed -- never exercised
    // before (the existing fixture-based tests only ever advance partway through).
    const char *const tourJson = R"({
        "id": "single-step-tour",
        "title": "Single Step",
        "steps": [ { "key": "only", "title": "Just look", "body": "Body" } ]
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "single_step.json", tourJson);

    const QString tourId = QStringLiteral("single-step-tour");
    Settings::setTourProgress(tourId, 0);
    QStringList completed = Settings::completedTours();
    completed.removeAll(tourId);
    Settings::setCompletedTours(completed);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    QSignalSpy completedSpy(&engine, &TourEngine::tourCompleted);
    QSignalSpy stepChangedSpy(&engine, &TourEngine::stepChanged);
    engine.advanceStep();

    QCOMPARE(completedSpy.count(), 1);
    QCOMPARE(stepChangedSpy.count(), 0); // reachedEnd path, not the advanced path
    QVERIFY(!engine.isActive());
    QVERIFY(Settings::completedTours().contains(tourId));
    QCOMPARE(Settings::tourProgress(tourId), 0);
}

void TestTourEngine::testLoadFromResourceMissingFileFails()
{
    TourEngine engine;
    QVERIFY(!engine.loadFromResource("/nonexistent/path/to/tour.json"));
    QVERIFY(!engine.isActive());
}

void TestTourEngine::testLoadFromResourceInvalidJsonFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "garbage.json", "this is not valid JSON {{{");

    TourEngine engine;
    QVERIFY(!engine.loadFromResource(path));
}

void TestTourEngine::testLoadFromResourceMissingIdOrTitleFails()
{
    const char *const tourJson = R"({
        "title": "No Id Here",
        "steps": [ { "title": "x", "body": "y" } ]
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "no_id.json", tourJson);

    TourEngine engine;
    QVERIFY(!engine.loadFromResource(path));
}

void TestTourEngine::testLoadFromResourceEmptyStepsArrayFails()
{
    const char *const tourJson = R"({
        "id": "empty-steps",
        "title": "Empty",
        "steps": []
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "empty_steps.json", tourJson);

    TourEngine engine;
    QVERIFY(!engine.loadFromResource(path));
}

void TestTourEngine::testStepWithoutKeyUsesRawTitleAndBodyDirectly()
{
    // Without a "key", fillTourStepFields() must use the raw JSON text directly rather than
    // looking it up in the (empty, for this made-up id) translation catalog.
    const char *const tourJson = R"({
        "id": "no-key-tour",
        "title": "No Key",
        "steps": [ { "title": "Raw title text", "body": "Raw body text" } ]
    })";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeJsonFile(dir, "no_key.json", tourJson);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    QCOMPARE(engine.totalSteps(), 1);
    QCOMPARE(engine.currentStepData().title, QStringLiteral("Raw title text"));
    QCOMPARE(engine.currentStepData().body, QStringLiteral("Raw body text"));
}

void TestTourEngine::testCurrentStepDataBeforeLoadReturnsEmptyStep()
{
    TourEngine engine;
    const TourStep &step = engine.currentStepData();

    QVERIFY(step.title.isEmpty());
    QVERIFY(step.body.isEmpty());
    QVERIFY(step.target.isEmpty());
}

void TestTourEngine::testRetranslateReloadFailureDeactivatesEngine()
{
    // If the underlying resource becomes unreadable between load and a later retranslate()
    // (e.g. a user-writable custom tour file deleted mid-session), loadFromResource()'s own
    // unconditional state reset at entry leaves the engine deactivated even though
    // retranslate() itself just returns false.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    QVERIFY(engine.isActive());

    QVERIFY(QFile::remove(path));

    QSignalSpy retranslatedSpy(&engine, &TourEngine::retranslated);
    engine.retranslate();

    QCOMPARE(retranslatedSpy.count(), 0);
    QVERIFY(!engine.isActive());
}
