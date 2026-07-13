// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestIC.h"

#include <QGraphicsSceneMouseEvent>
#include <QSignalSpy>

#include "App/Core/Settings.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
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
