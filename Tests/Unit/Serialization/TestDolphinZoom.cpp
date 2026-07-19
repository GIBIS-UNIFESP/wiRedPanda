// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinZoom.h"

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
