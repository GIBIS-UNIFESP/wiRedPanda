// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestIC.h"

#include <limits>
#include <memory>

#include <QGraphicsSceneMouseEvent>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Core/Settings.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Element/ICRenderer.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestICUnit::testICLoadFromFile()
{
    IC ic;
    QCOMPARE(ic.elementType(), ElementType::IC);
}

void TestICUnit::testICPortLabelResolution()
{
    IC ic;
    // Unloaded IC has no ports
    QVERIFY(ic.inputs().isEmpty());
    QVERIFY(ic.outputs().isEmpty());
}

void TestICUnit::testICNestedSaveLoad()
{
    IC ic;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    ic.save(stream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}

void TestICUnit::testICInvalidFile()
{
    IC ic;
    QVERIFY(ic.file().isEmpty());
}

void TestICUnit::testICPreviewPopupRespectsDisabledSetting()
{
    const bool originalIcPreviewDisabled = Settings::icPreviewDisabled();

    IC ic;
    ICPreviewPopup popup;

    Settings::setIcPreviewDisabled(true);
    popup.showForIC(&ic, QPoint(0, 0));
    QVERIFY(popup.pendingIC() == nullptr);
    QVERIFY(!popup.isShowActiveFor(&ic));

    Settings::setIcPreviewDisabled(false);
    popup.showForIC(&ic, QPoint(0, 0));
    QCOMPARE(popup.pendingIC(), &ic);
    QVERIFY(popup.isShowActiveFor(&ic));

    Settings::setIcPreviewDisabled(originalIcPreviewDisabled);
}

void TestICUnit::testPreviewPopupClampsToScreen()
{
    const QRect screen(0, 0, 1920, 1080);
    const QSize popup(500, 350);

    // Away from any edge: the popup sits just down-right of the cursor.
    QCOMPARE(ICPreviewPopup::clampedPopupPos(QPoint(800, 400), popup, screen), QPoint(816, 416));

    // Cursor in the bottom-right corner — the case the old screenAt(offset) lookup rendered
    // off-screen: the whole popup must still fit within the screen.
    const QPoint atCorner = ICPreviewPopup::clampedPopupPos(QPoint(1915, 1075), popup, screen);
    QVERIFY(screen.contains(atCorner));
    QVERIFY(screen.contains(atCorner + QPoint(popup.width() - 1, popup.height() - 1)));

    // A secondary monitor offset from the origin: clamping respects its geometry, not (0,0).
    const QRect second(1920, 0, 1920, 1080);
    const QPoint onSecond = ICPreviewPopup::clampedPopupPos(QPoint(3835, 500), popup, second);
    QVERIFY(onSecond.x() >= second.left());
    QVERIFY(second.contains(onSecond + QPoint(popup.width() - 1, popup.height() - 1)));
}

void TestICUnit::testDoubleClickOpensSubCircuitNotInlineEditor()
{
    WorkSpace workspace;
    auto *ic = new IC;
    QVERIFY(ic->hasLabel()); // otherwise this test wouldn't distinguish the two code paths
    workspace.scene()->addItem(ic);

    QSignalSpy openSpy(ic, &IC::requestOpenSubCircuit);
    QSignalSpy inlineEditSpy(ic, &GraphicElement::inlineEditRequested);

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseDoubleClick);
    event.setScenePos(ic->mapToScene(ic->boundingRect().center()));
    event.setButton(Qt::LeftButton);
    event.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(workspace.scene(), &event);

    QCOMPARE(openSpy.count(), 1);
    QCOMPARE(inlineEditSpy.count(), 0);
}

void TestICUnit::testUnloadedIcSimulationMethodsAreNoOps()
{
    // A freshly-constructed, never-loaded IC has 0 internal elements -- ICSimulation::initialize()/
    // update()/resettle() must all recognize this and return immediately rather than operate on
    // an empty m_sortedInternalElements/m_internalElements.
    IC ic;
    ic.initializeSimulation();
    ic.updateLogic();
    ic.resettleCombinational();

    QCOMPARE(ic.inputSize(), 0);
    QCOMPARE(ic.outputSize(), 0);
}

void TestICUnit::testLoadedIcWithDisconnectedInputIsUnknown()
{
    // Build a tiny real sub-circuit (the established WorkSpace::save() + IC::loadFile()
    // technique) so the loaded IC has genuine internal elements and m_sortedInternalElements is
    // non-empty, then drive it with its own input port left disconnected, exercising
    // ICSimulation::update()'s/resettle()'s "!simUpdateInputsAllowUnknown()" guard.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    WorkSpace subWorkspace;
    CircuitBuilder subBuilder(subWorkspace.scene());
    InputSwitch sw;
    Led led;
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);

    const QString subPath = subDir.path() + "/disconnected_input_probe.panda";
    QCOMPARE(subWorkspace.save(subPath), WorkSpace::SaveOutcome::Saved);

    auto ic = std::make_unique<IC>();
    ic->loadFile(subPath, subDir.path());
    QVERIFY(ic->inputSize() >= 1);
    QVERIFY(ic->outputSize() >= 1);

    // ICLoader derives an input port's default status from its internal boundary element's
    // status *at load time* (Inactive here, since InputSwitch defaults off) -- force it to
    // Unknown directly so the disconnected port genuinely fails simUpdateInputsAllowUnknown(),
    // rather than "successfully" resolving to a definite level nothing actually drove.
    ic->inputPort(0)->setDefaultStatus(Status::Unknown);

    TestUtils::initElm(*ic); // allocate the IC's own sim vectors without connecting its ports
    ic->initializeSimulation();

    ic->updateLogic();
    QCOMPARE(ic->outputValue(0), Status::Unknown);

    ic->resettleCombinational();
    QCOMPARE(ic->outputValue(0), Status::Unknown);
}

void TestICUnit::testGeneratePixmapWithNonFiniteBoundsIsNoOp()
{
    // Defense-in-depth: a non-finite port position makes renderBodyBounds() NaN, and
    // QSizeF::toSize() asserts (!qIsNaN) on that -> process abort. The load-side guards
    // (validateFinitePos/validateFiniteAngle) reject this on a real load, so it's forced
    // directly via the port's own public setPos() to exercise the guard deterministically.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());
    WorkSpace subWorkspace;
    CircuitBuilder subBuilder(subWorkspace.scene());
    InputSwitch sw;
    Led led;
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);
    const QString subPath = subDir.path() + "/nonfinite_pixmap_probe.panda";
    QCOMPARE(subWorkspace.save(subPath), WorkSpace::SaveOutcome::Saved);

    auto ic = std::make_unique<IC>();
    ic->loadFile(subPath, subDir.path());
    QVERIFY(ic->inputSize() >= 1);

    ic->inputPort(0)->setPos(std::numeric_limits<qreal>::quiet_NaN(), 0);
    ICRenderer::generatePixmap(*ic); // must return early rather than abort
}

void TestICUnit::testGeneratePreviewPixmapWithNonFiniteBoundsIsNoOp()
{
    IC ic;
    auto elm = std::make_unique<And>();
    elm->setPos(std::numeric_limits<qreal>::quiet_NaN(), 0);

    QList<QGraphicsItem *> items{elm.get()};
    ICRenderer::generatePreviewPixmap(ic, items);

    QVERIFY2(ic.previewPixmap().isNull(), "a non-finite item bounding rect must reset the preview to null, not abort");
}

void TestICUnit::testGeneratePreviewPixmapWithDegenerateAspectRatioIsEmpty()
{
    // An extremely wide/thin circuit layout scales down to a QSize with one dimension
    // rounding to 0 (empty), which must reset the preview to null rather than construct a
    // degenerate QPixmap.
    IC ic;
    auto elm1 = std::make_unique<And>();
    auto elm2 = std::make_unique<And>();
    elm1->setPos(0, 0);
    elm2->setPos(100000000, 0);

    QList<QGraphicsItem *> items{elm1.get(), elm2.get()};
    ICRenderer::generatePreviewPixmap(ic, items);

    QVERIFY(ic.previewPixmap().isNull());
}
