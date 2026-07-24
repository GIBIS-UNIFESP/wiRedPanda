// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestSignalDelegate.h"

#include <QImage>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "App/BeWavedDolphin/SignalDelegate.h"
#include "App/BeWavedDolphin/SignalModel.h"

namespace {

QImage renderCell(SignalDelegate &delegate, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QImage image(option.rect.width(), option.rect.height(), QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    delegate.paint(&painter, option, index);
    painter.end();
    return image;
}

} // namespace

void TestSignalDelegate::testPaintNumberModeDiffersFromLineMode()
{
    SignalModel model(1, 2);
    model.setInputRows(1);
    model.setValue(0, 0, 1);
    model.setValue(0, 1, 1);

    SignalDelegate delegate;
    QCOMPARE(delegate.plotType(), PlotType::Line); // default

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 40, 30);

    delegate.setPlotType(PlotType::Line);
    const QImage lineImage = renderCell(delegate, option, model.index(0, 1));

    delegate.setPlotType(PlotType::Number);
    QCOMPARE(delegate.plotType(), PlotType::Number);
    const QImage numberImage = renderCell(delegate, option, model.index(0, 1));

    QVERIFY2(lineImage != numberImage, "Number mode should render differently from waveform mode");
}

void TestSignalDelegate::testPaintSelectedCellDrawsHighlightBehindWaveform()
{
    SignalModel model(1, 2);
    model.setInputRows(1);
    model.setValue(0, 0, 1);
    model.setValue(0, 1, 1);

    SignalDelegate delegate;
    delegate.setPlotType(PlotType::Line);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 40, 30);
    QPalette palette;
    palette.setColor(QPalette::Highlight, Qt::red);
    option.palette = palette;

    option.state.setFlag(QStyle::State_Selected, false);
    const QImage unselected = renderCell(delegate, option, model.index(0, 1));

    option.state.setFlag(QStyle::State_Selected, true);
    const QImage selected = renderCell(delegate, option, model.index(0, 1));

    QVERIFY2(unselected != selected, "A selected cell must paint the highlight behind the waveform");
}
