// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Tour/TestTourEngine.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QSignalSpy>
#include <QTemporaryDir>

#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>

#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/Tour/TourEngine.h"
#include "App/Tour/TourOverlay.h"
#include "Tests/Common/TestUtils.h"

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

void TestTourEngine::testOverlayConstructionBuildsUiWidgets()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    QVERIFY(overlay.m_callout != nullptr);
    QVERIFY(overlay.m_stepCounter != nullptr);
    QVERIFY(overlay.m_titleLabel != nullptr);
    QVERIFY(overlay.m_bodyLabel != nullptr);
    QVERIFY(overlay.m_prevButton != nullptr);
    QVERIFY(overlay.m_nextButton != nullptr);
    QVERIFY(overlay.m_closeButton != nullptr);
    QCOMPARE(overlay.m_callout->width(), 360);
}

void TestTourEngine::testOverlayAppliesThemeReactsToThemeChange()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    ThemeManager::setTheme(Theme::Light);
    const QColor lightDim = overlay.m_dimColor;
    const QColor lightSpotlight = overlay.m_spotlightColor;

    ThemeManager::setTheme(Theme::Dark);
    const QColor darkDim = overlay.m_dimColor;
    const QColor darkSpotlight = overlay.m_spotlightColor;

    QVERIFY2(lightDim != darkDim, "applyTheme() must pick different dim colors for light vs dark");
    QVERIFY2(lightSpotlight != darkSpotlight, "applyTheme() must pick different spotlight colors for light vs dark");

    ThemeManager::setTheme(Theme::Light);
}

void TestTourEngine::testOverlayOnStepChangedUpdatesLabelsAndButtons()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    TourStep step;
    step.title = QStringLiteral("Step Title");
    step.body = QStringLiteral("Step body text");

    overlay.onStepChanged(0, 2, step);

    QCOMPARE(overlay.m_titleLabel->text(), QStringLiteral("Step Title"));
    QCOMPARE(overlay.m_bodyLabel->text(), QStringLiteral("Step body text"));
    QVERIFY(!overlay.m_prevButton->isEnabled()); // step 0
}

void TestTourEngine::testOverlayOnStepChangedEnablesPrevAfterFirstStep()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    TourStep step;
    overlay.onStepChanged(1, 3, step);

    QVERIFY(overlay.m_prevButton->isEnabled());
}

void TestTourEngine::testOverlayOnStepChangedLastStepShowsFinish()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    TourStep step;
    overlay.onStepChanged(2, 3, step); // step == total - 1

    QCOMPARE(overlay.m_nextButton->text(), QStringLiteral("Finish"));
}

void TestTourEngine::testOverlayOnStepChangedSkipsResolverForEmptyOrNoneTarget()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    bool resolverCalled = false;
    overlay.setTargetResolver([&](const QString &) {
        resolverCalled = true;
        return QRect(0, 0, 10, 10);
    });

    TourStep emptyTargetStep;
    overlay.onStepChanged(0, 1, emptyTargetStep);
    QVERIFY2(!resolverCalled, "An empty target must not call the resolver");
    QVERIFY(overlay.m_highlightRect.isEmpty());

    TourStep noneTargetStep;
    noneTargetStep.target = QStringLiteral("none");
    overlay.onStepChanged(0, 1, noneTargetStep);
    QVERIFY2(!resolverCalled, "A \"none\" target must not call the resolver either");
    QVERIFY(overlay.m_highlightRect.isEmpty());
}

void TestTourEngine::testOverlayOnStepChangedResolvesHighlightRectForRealTarget()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    const QRect resolved(15, 25, 100, 40);
    overlay.setTargetResolver([&](const QString &targetId) {
        Q_UNUSED(targetId);
        return resolved;
    });

    TourStep step;
    step.target = QStringLiteral("some-widget");
    overlay.onStepChanged(0, 1, step);

    QCOMPARE(overlay.m_highlightRect, resolved);
    QCOMPARE(overlay.m_currentTarget, QStringLiteral("some-widget"));
}

void TestTourEngine::testOverlayOnRetranslatedUpdatesLabelsAndButtons()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    TourOverlay overlay(&engine);
    overlay.onRetranslated();

    QCOMPARE(overlay.m_titleLabel->text(), engine.currentStepData().title);
    QCOMPARE(overlay.m_bodyLabel->text(), engine.currentStepData().body);
    QCOMPARE(overlay.m_stepCounter->text(), QStringLiteral("Step 1 of 2"));
}

void TestTourEngine::testOverlayOnTourFinishedHidesAndEmitsCloseRequested()
{
    TourEngine engine;
    TourOverlay overlay(&engine);
    overlay.show();

    QSignalSpy closeSpy(&overlay, &TourOverlay::closeRequested);
    overlay.onTourFinished();

    QVERIFY(overlay.isHidden());
    QCOMPARE(closeSpy.count(), 1);
}

void TestTourEngine::testOverlaySetParentWindowReparentsAndResizes()
{
    // parentA/parentB must be declared (and so destroyed, in reverse order) before overlay:
    // once reparented to parentB, Qt's ownership tree makes parentB responsible for deleting
    // overlay, so overlay must be destructed first at scope exit -- otherwise parentB's
    // destructor tries to delete an already-destructed stack object.
    TourEngine engine;
    QWidget parentA;
    parentA.resize(400, 300);
    QWidget parentB;
    parentB.resize(600, 500);

    TourOverlay overlay(&engine, &parentA);
    QCOMPARE(overlay.size(), parentA.size());

    overlay.setParentWindow(&parentB);

    QCOMPARE(overlay.parentWidget(), &parentB);
    QCOMPARE(overlay.size(), parentB.size());

    // The old parent's filter must really be uninstalled: resizing it must not resize the
    // overlay (now reparented) any more.
    parentA.resize(100, 100);
    QCOMPARE(overlay.size(), parentB.size());
}

void TestTourEngine::testOverlayEventFilterResizesOnParentResize()
{
    TourEngine engine;
    QWidget parent;
    parent.resize(400, 300);
    TourOverlay overlay(&engine, &parent);

    QResizeEvent event(QSize(500, 350), parent.size());
    parent.resize(500, 350);
    QApplication::sendEvent(&parent, &event);

    QCOMPARE(overlay.size(), QSize(500, 350));
}

void TestTourEngine::testOverlayPrevButtonCallsGoToPreviousStep()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();
    engine.advanceStep();
    QCOMPARE(engine.currentStep(), 1);

    TourOverlay overlay(&engine);
    overlay.m_prevButton->click();

    QCOMPARE(engine.currentStep(), 0);
}

void TestTourEngine::testOverlayNextButtonCallsAdvanceStep()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeTourFixture(dir);

    TourEngine engine;
    QVERIFY(engine.loadFromResource(path));
    engine.start();

    TourOverlay overlay(&engine);
    overlay.m_nextButton->click();

    QCOMPARE(engine.currentStep(), 1);
}

void TestTourEngine::testOverlayCloseButtonEmitsCloseRequested()
{
    TourEngine engine;
    TourOverlay overlay(&engine);

    QSignalSpy closeSpy(&overlay, &TourOverlay::closeRequested);
    overlay.m_closeButton->click();

    QCOMPARE(closeSpy.count(), 1);
}

void TestTourEngine::testRepositionCalloutCentersWhenNoHighlight()
{
    TourEngine engine;
    TourOverlay overlay(&engine);
    overlay.resize(400, 300);

    TourStep step; // no target -- m_highlightRect stays empty
    overlay.onStepChanged(0, 1, step);

    const int expectedX = (overlay.width() - overlay.m_callout->width()) / 2;
    const int expectedY = (overlay.height() - overlay.m_callout->height()) / 2;
    QCOMPARE(overlay.m_callout->pos(), QPoint(expectedX, expectedY));
}

void TestTourEngine::testRepositionCalloutPositionsBelowSpotlight()
{
    TourEngine engine;
    TourOverlay overlay(&engine);
    overlay.resize(800, 600);

    // A small spotlight near the top, with plenty of room below for the callout.
    overlay.setTargetResolver([](const QString &) { return QRect(100, 50, 40, 40); });
    TourStep step;
    step.target = QStringLiteral("top-widget");
    overlay.onStepChanged(0, 1, step);

    QVERIFY2(overlay.m_callout->y() > 50 + 40, "The callout must be positioned below the spotlight");
}

void TestTourEngine::testRepositionCalloutPositionsAboveWhenNoRoomBelow()
{
    TourEngine engine;
    TourOverlay overlay(&engine);
    overlay.resize(800, 600);

    // A spotlight near the bottom of the overlay leaves no room below for the callout.
    overlay.setTargetResolver([](const QString &) { return QRect(100, 560, 40, 30); });
    TourStep step;
    step.target = QStringLiteral("bottom-widget");
    overlay.onStepChanged(0, 1, step);

    QVERIFY2(overlay.m_callout->y() + overlay.m_callout->height() <= 560,
              "The callout must be positioned above the spotlight when there's no room below");
}

void TestTourEngine::testRepositionCalloutCentersRightWhenNoRoomAboveOrBelow()
{
    TourEngine engine;
    TourOverlay overlay(&engine);
    // A spotlight spanning the whole overlay height leaves no room either above or below it,
    // regardless of the overlay's own size -- keep the overlay tall enough that the callout
    // itself still fits (qBound(pad, cy, oh - ch - pad) would assert otherwise).
    overlay.resize(800, 600);

    overlay.setTargetResolver([](const QString &) { return QRect(50, 0, 40, 600); });
    TourStep step;
    step.target = QStringLiteral("full-height-widget");
    overlay.onStepChanged(0, 1, step);

    // Vertically centered, shifted to the right of (or left of, if that doesn't fit either)
    // the spotlight rather than above/below it.
    const int expectedY = (overlay.height() - overlay.m_callout->height()) / 2;
    QCOMPARE(overlay.m_callout->y(), expectedY);
}

void TestTourEngine::testPaintEventDrawsWithoutHighlight()
{
    TourEngine engine;
    TourOverlay overlay(&engine);
    overlay.resize(400, 300);

    const QPixmap pixmap = overlay.grab();
    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "paintEvent() must dim the whole overlay even with no highlight");
}

void TestTourEngine::testPaintEventReResolvesHighlightRectFromResolver()
{
    TourEngine engine;
    TourOverlay overlay(&engine);
    overlay.resize(400, 300);

    QRect resolvedRect(10, 10, 50, 50);
    overlay.setTargetResolver([&](const QString &) { return resolvedRect; });

    TourStep step;
    step.target = QStringLiteral("widget-a");
    overlay.onStepChanged(0, 1, step);
    QCOMPARE(overlay.m_highlightRect, resolvedRect);

    // Simulate the spotlighted widget having moved since onStepChanged() last resolved it --
    // paintEvent() must re-resolve and reposition the callout to match.
    resolvedRect = QRect(20, 20, 60, 60);
    const QPixmap pixmap = overlay.grab();

    QCOMPARE(overlay.m_highlightRect, resolvedRect);
    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "paintEvent() must draw a visible spotlight outline");
}
