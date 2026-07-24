// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestSceneUiBinder.h"

#include <memory>

#include <QMainWindow>
#include <QMenu>
#include <QSignalSpy>
#include <QStatusBar>
#include <QTableWidget>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/ElementEditor.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/SceneUiBinder.h"
#include "Tests/Common/TestUtils.h"

namespace {

// Builds the trio of real, directly-constructible dependencies SceneUiBinder needs, mirroring
// TestElementPalette's QMainWindow + MainWindowUi::setupUi() pattern. palette/binder are
// heap-allocated and built in the constructor BODY (not as direct value members) because
// their own constructors dereference ui's widget pointers, which setupUi() must populate
// first -- member-initializer-list order can't be relied on for that sequencing.
struct SceneUiBinderFixture {
    QMainWindow window;
    MainWindowUi ui;
    ICPreviewPopup previewPopup;
    std::unique_ptr<ElementPalette> palette;
    std::unique_ptr<SceneUiBinder> binder;

    SceneUiBinderFixture()
    {
        ui.setupUi(&window);
        palette = std::make_unique<ElementPalette>(&ui);
        binder = std::make_unique<SceneUiBinder>(&ui, palette.get(), &previewPopup, &window);
    }
};

} // namespace

void TestSceneUiBinder::testGuardMethodsAreNoOpBeforeAnyBind()
{
    SceneUiBinderFixture f;

    const auto actionsBefore = f.ui.menuEdit->actions().size();
    const auto toolBarActionsBefore = f.ui.mainToolBar->actions().size();

    // Friend access (App/UI/SceneUiBinder.h): none of these have ever had bind() called, so
    // m_bound is still null -- each must decline rather than dereference it.
    f.binder->updateStatusInfo();
    QVERIFY(f.binder->m_statusInfo->text().isEmpty());

    f.binder->addUndoRedoMenu();
    f.binder->removeUndoRedoMenu();
    QCOMPARE(f.ui.menuEdit->actions().size(), actionsBefore);
    QCOMPARE(f.ui.mainToolBar->actions().size(), toolBarActionsBefore);

    const bool zoomInEnabledBefore = f.ui.actionZoomIn->isEnabled();
    const bool zoomOutEnabledBefore = f.ui.actionZoomOut->isEnabled();
    f.binder->syncZoomActions();
    QCOMPARE(f.ui.actionZoomIn->isEnabled(), zoomInEnabledBefore);
    QCOMPARE(f.ui.actionZoomOut->isEnabled(), zoomOutEnabledBefore);
}

void TestSceneUiBinder::testBindWithTooFewMenuActionsSkipsUndoRedoInsertionAndRemoval()
{
    SceneUiBinderFixture f;
    WorkSpace tab;

    // Replace the real (10+ action) menuEdit with a fresh, empty one so bind()'s internal
    // addUndoRedoMenu() takes its "too few actions" early return instead of inserting.
    auto *shortMenu = new QMenu(&f.window);
    f.ui.menuEdit = shortMenu;

    f.binder->bind(&tab);
    QCOMPARE(shortMenu->actions().size(), 0);

    // unbind()'s internal removeUndoRedoMenu() must likewise decline (nothing was ever
    // inserted to remove) rather than indexing into an empty action list.
    f.binder->unbind();
    QCOMPARE(shortMenu->actions().size(), 0);
}

void TestSceneUiBinder::testBindConnectsSimulationWarningToStatusBar()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    emit tab.simulation()->simulationWarning(QStringLiteral("feedback did not converge"));

    QCOMPARE(f.ui.statusBar->currentMessage(), QStringLiteral("feedback did not converge"));

    f.binder->unbind();
}

void TestSceneUiBinder::testBindConnectsShowStatusMessageRequestedToStatusBar()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    emit tab.scene()->showStatusMessageRequested(QStringLiteral("wire rejected"));

    QCOMPARE(f.ui.statusBar->currentMessage(), QStringLiteral("wire rejected"));

    f.binder->unbind();
}

void TestSceneUiBinder::testBindConnectsOpenTruthTableRequestedToElementEditor()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    auto *tt = static_cast<TruthTable *>(ElementFactory::buildElement(ElementType::TruthTable));
    tab.scene()->addItem(tt);
    tt->setSelected(true);
    QCoreApplication::processEvents(); // let ElementEditor::selectionChanged() pick it up

    emit tab.scene()->openTruthTableRequested();

    auto *table = f.ui.elementEditor->findChild<QTableWidget *>();
    QVERIFY(table);
    QCOMPARE(table->rowCount(), static_cast<int>(std::pow(2, tt->inputSize())));

    f.binder->unbind();
}

void TestSceneUiBinder::testBindForwardsIcOpenRequestedWithBlobName()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    QSignalSpy openIcSpy(f.binder.get(), &SceneUiBinder::openICRequested);
    QSignalSpy loadFileSpy(f.binder.get(), &SceneUiBinder::loadFileRequested);

    emit tab.scene()->icOpenRequested(7, QStringLiteral("some_blob"), QString());

    QCOMPARE(openIcSpy.count(), 1);
    QCOMPARE(openIcSpy.constFirst().at(0).toString(), QStringLiteral("some_blob"));
    QCOMPARE(openIcSpy.constFirst().at(1).toInt(), 7);
    QCOMPARE(loadFileSpy.count(), 0);

    f.binder->unbind();
}

void TestSceneUiBinder::testBindForwardsIcOpenRequestedWithFilePathOnly()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    QSignalSpy openIcSpy(f.binder.get(), &SceneUiBinder::openICRequested);
    QSignalSpy loadFileSpy(f.binder.get(), &SceneUiBinder::loadFileRequested);

    emit tab.scene()->icOpenRequested(0, QString(), QStringLiteral("counter.panda"));

    QCOMPARE(loadFileSpy.count(), 1);
    QCOMPARE(loadFileSpy.constFirst().at(0).toString(), QStringLiteral("counter.panda"));
    QCOMPARE(openIcSpy.count(), 0);

    f.binder->unbind();
}

void TestSceneUiBinder::testBindForwardsFileDropRequestedToLoadFileRequested()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    QSignalSpy loadFileSpy(f.binder.get(), &SceneUiBinder::loadFileRequested);

    emit tab.scene()->fileDropRequested(QStringLiteral("dropped.panda"));

    QCOMPARE(loadFileSpy.count(), 1);
    QCOMPARE(loadFileSpy.constFirst().at(0).toString(), QStringLiteral("dropped.panda"));

    f.binder->unbind();
}

void TestSceneUiBinder::testBindShowsIcPreviewOnPreviewRequestedAndOnMoveWhenNotAlreadyActive()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    auto *ic = static_cast<IC *>(ElementFactory::buildElement(ElementType::IC));

    emit tab.scene()->icPreviewRequested(ic, QPoint(10, 10));
    QCOMPARE(f.previewPopup.pendingIC(), ic);

    // A second, different IC is not yet "active" (nothing shown/pending for it specifically
    // was armed by a matching showForIC call), so icPreviewMoved's else-branch (showForIC)
    // must fire rather than the isShowActiveFor() early branch.
    auto *ic2 = static_cast<IC *>(ElementFactory::buildElement(ElementType::IC));
    QVERIFY(!f.previewPopup.isShowActiveFor(ic2));
    emit tab.scene()->icPreviewMoved(ic2, QPoint(20, 20));
    QCOMPARE(f.previewPopup.pendingIC(), ic2);

    f.binder->unbind();
    delete ic;
    delete ic2;
}

void TestSceneUiBinder::testBindCancelsIcPreviewOnCancelRequested()
{
    SceneUiBinderFixture f;
    WorkSpace tab;
    f.binder->bind(&tab);

    auto *ic = static_cast<IC *>(ElementFactory::buildElement(ElementType::IC));

    // Force the popup visible directly (bypassing the real hover-timer/render pipeline,
    // which isn't what this test is about) so cancelling has an observable effect.
    f.previewPopup.show();
    QVERIFY(f.previewPopup.isVisible());

    emit tab.scene()->icPreviewCancelRequested(ic);

    QVERIFY(!f.previewPopup.isVisible());

    f.binder->unbind();
    delete ic;
}
