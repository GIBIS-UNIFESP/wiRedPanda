// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Exercise/TestExerciseEngine.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Core/Settings.h"
#include "App/Exercise/ExerciseEngine.h"

namespace {

const char *const kExerciseFixtureId = "test-retranslate-exercise";

const char *const kExerciseFixtureJson = R"({
    "id": "test-retranslate-exercise",
    "title": "Test Exercise",
    "steps": [
        { "key": "step0", "instruction": "Instruction 0", "hint": "Hint 0" },
        { "key": "step1", "instruction": "Instruction 1", "hint": "Hint 1" }
    ]
})";

QString writeExerciseFixture(const QTemporaryDir &dir)
{
    const QString path = dir.filePath("exercise.json");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    file.write(kExerciseFixtureJson);
    return path;
}

} // namespace

void TestExerciseEngine::testRetranslateBeforeLoadIsNoOp()
{
    ExerciseEngine engine;
    QSignalSpy retranslatedSpy(&engine, &ExerciseEngine::retranslated);

    engine.retranslate();

    QVERIFY(!engine.isActive());
    QCOMPARE(retranslatedSpy.count(), 0);
}

void TestExerciseEngine::testRetranslateWhileInactiveIsNoOp()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    QVERIFY(!engine.isActive()); // loadFromResource() alone never activates the engine

    QSignalSpy retranslatedSpy(&engine, &ExerciseEngine::retranslated);
    engine.retranslate();

    QVERIFY(!engine.isActive());
    QCOMPARE(retranslatedSpy.count(), 0);
}

void TestExerciseEngine::testRetranslatePreservesProgressAndData()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("en");
    Settings::setExerciseProgress(QLatin1String(kExerciseFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();

    QCOMPARE(engine.currentStep(), 1);
    QVERIFY(engine.isActive());

    engine.retranslate();

    QCOMPARE(engine.currentStep(), 1);
    QVERIFY(engine.isActive());
    QCOMPARE(engine.exerciseTitle(), QStringLiteral("Test Exercise"));
    QCOMPARE(engine.currentStepData().instruction, QStringLiteral("Instruction 1"));
    QCOMPARE(engine.currentStepData().hint, QStringLiteral("Hint 1"));

    Settings::setLanguage(originalLang);
}

void TestExerciseEngine::testRetranslateEmitsRetranslatedOnly()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("en");
    Settings::setExerciseProgress(QLatin1String(kExerciseFixtureId), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeExerciseFixture(dir);

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    QSignalSpy retranslatedSpy(&engine, &ExerciseEngine::retranslated);
    QSignalSpy stepChangedSpy(&engine, &ExerciseEngine::stepChanged);
    QSignalSpy stepCompletedSpy(&engine, &ExerciseEngine::stepCompleted);

    engine.retranslate();

    QCOMPARE(retranslatedSpy.count(), 1);
    QCOMPARE(stepChangedSpy.count(), 0);
    QCOMPARE(stepCompletedSpy.count(), 0);

    Settings::setLanguage(originalLang);
}

void TestExerciseEngine::testNegativeMinCountClampsToZero()
{
    // Regression: a negative minCount (typo'd or malicious custom exercise JSON — Exercise/Tour
    // content is end-user-writable) silently auto-satisfied "count < minCount" since any
    // non-negative count is always >= a negative number, permanently skipping the check.
    const char *const json = R"({
        "id": "test-negative-mincount",
        "title": "Test",
        "steps": [
            {
                "key": "step0",
                "instruction": "Instruction",
                "requiredElements": [ { "type": "And", "minCount": -5 } ],
                "requiredConnections": [ { "fromType": "And", "toType": "Or", "minCount": -3 } ]
            }
        ]
    })";

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("negative_mincount.json");
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(json);
    }

    ExerciseEngine engine;
    QVERIFY(engine.loadFromResource(path));

    const ExerciseStep &step = engine.currentStepData();
    QCOMPARE(step.requiredElements.size(), 1);
    QCOMPARE(step.requiredElements[0].minCount, 0);
    QCOMPARE(step.requiredConnections.size(), 1);
    QCOMPARE(step.requiredConnections[0].minCount, 0);
}
