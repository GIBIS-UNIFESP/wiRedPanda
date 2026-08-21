// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinZoom.h"

#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTableView>

#include "App/BeWavedDolphin/DolphinZoom.h"

void TestDolphinZoom::testFitScreenNoOpOnEmptyModel()
{
    QTableView view;
    QStandardItemModel model(0, 5); // zero rows
    view.setModel(&model);

    DolphinZoom zoom(&view);
    QCOMPARE(zoom.zoomLevel(), 0);
    QCOMPARE(zoom.fitScale(), 1.0);

    zoom.fitScreen();

    QCOMPARE(zoom.zoomLevel(), 0);
    QCOMPARE(zoom.fitScale(), 1.0);
}

void TestDolphinZoom::testFitScreenNoOpOnDegenerateViewportGeometry()
{
    QTableView view;
    QStandardItemModel model(1, 1);
    // A very wide row label makes the vertical header's own size hint exceed the whole
    // (deliberately tiny) view interior, driving the fit computation non-positive.
    model.setVerticalHeaderItem(0, new QStandardItem(QString(500, QChar('W'))));
    view.setModel(&model);
    view.resize(1, 1);

    DolphinZoom zoom(&view);
    const double scaleBefore = zoom.fitScale();
    const int levelBefore = zoom.zoomLevel();

    zoom.fitScreen();

    // Degenerate geometry must leave the zoom untouched rather than computing a garbage scale.
    QCOMPARE(zoom.fitScale(), scaleBefore);
    QCOMPARE(zoom.zoomLevel(), levelBefore);
}

void TestDolphinZoom::testZoomInWidensColumnsButNotRows()
{
    // Column-zoom (apply()'s colScale = fitScale * kZoomStep^level) widens columns only;
    // Fit Screen's uniform scale (left at the 1.0 baseline here) is what would touch rows.
    QTableView view;
    QStandardItemModel model(1, 1);
    view.setModel(&model);

    DolphinZoom zoom(&view);
    zoom.reset(); // establish the DolphinZoom-computed baseline via apply(), not QHeaderView's own default
    const int colWidthBefore = view.horizontalHeader()->defaultSectionSize();
    const int rowHeightBefore = view.verticalHeader()->defaultSectionSize();

    zoom.zoomIn();

    QCOMPARE(zoom.zoomLevel(), 1);
    QVERIFY2(view.horizontalHeader()->defaultSectionSize() > colWidthBefore,
             "zoomIn() must widen the column section size");
    QCOMPARE(view.verticalHeader()->defaultSectionSize(), rowHeightBefore);

    const int widthAtLevel1 = view.horizontalHeader()->defaultSectionSize();
    zoom.zoomIn();

    QCOMPARE(zoom.zoomLevel(), 2);
    QVERIFY2(view.horizontalHeader()->defaultSectionSize() > widthAtLevel1,
             "A second zoomIn() must widen the column section size further");
}

void TestDolphinZoom::testZoomOutReturnsColumnsToBaselineWidth()
{
    QTableView view;
    QStandardItemModel model(1, 1);
    view.setModel(&model);

    DolphinZoom zoom(&view);
    zoom.reset();
    const int baselineWidth = view.horizontalHeader()->defaultSectionSize();

    zoom.zoomIn();
    zoom.zoomIn();
    QVERIFY(view.horizontalHeader()->defaultSectionSize() > baselineWidth);

    zoom.zoomOut();
    zoom.zoomOut();

    QCOMPARE(zoom.zoomLevel(), 0);
    QCOMPARE(view.horizontalHeader()->defaultSectionSize(), baselineWidth);

    // zoomOut() at the baseline must not floor below level 0.
    zoom.zoomOut();
    QCOMPARE(zoom.zoomLevel(), 0);
    QCOMPARE(view.horizontalHeader()->defaultSectionSize(), baselineWidth);
}

void TestDolphinZoom::testCanZoomInFalseAtMaxLevel()
{
    QTableView view;
    QStandardItemModel model(1, 1);
    view.setModel(&model);

    DolphinZoom zoom(&view);
    QVERIFY(zoom.canZoomIn());

    // zoomIn() is internally capped, so calling it far past the real max is safe and must
    // still leave canZoomIn() false, not throw or wrap around.
    for (int i = 0; i < 20; ++i) {
        zoom.zoomIn();
    }

    QVERIFY2(!zoom.canZoomIn(), "canZoomIn() must be false once the max zoom level is reached");
}

void TestDolphinZoom::testCanZoomOutFalseAtBaseline()
{
    QTableView view;
    QStandardItemModel model(1, 1);
    view.setModel(&model);

    DolphinZoom zoom(&view);
    QVERIFY2(!zoom.canZoomOut(), "canZoomOut() must be false at the baseline zoom level");

    zoom.zoomIn();
    QVERIFY2(zoom.canZoomOut(), "canZoomOut() must be true once zoomed in past the baseline");

    zoom.zoomOut();
    QVERIFY2(!zoom.canZoomOut(), "canZoomOut() must be false again after returning to the baseline");
}

void TestDolphinZoom::testResetRestoresLevelAndScaleToBaseline()
{
    QTableView view;
    QStandardItemModel model(1, 1);
    view.setModel(&model);

    DolphinZoom zoom(&view);
    zoom.reset();
    const int baselineWidth = view.horizontalHeader()->defaultSectionSize();

    zoom.zoomIn();
    zoom.zoomIn();
    QVERIFY(zoom.zoomLevel() > 0);

    zoom.reset();

    QCOMPARE(zoom.zoomLevel(), 0);
    QCOMPARE(zoom.fitScale(), 1.0);
    QCOMPARE(view.horizontalHeader()->defaultSectionSize(), baselineWidth);
}
