// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestSignalDelegate.h"

#include <QImage>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "App/BeWavedDolphin/SignalDelegate.h"
#include "App/BeWavedDolphin/SignalModel.h"

Q_DECLARE_METATYPE(WaveSegment)

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

void TestSignalDelegate::testSegmentForCoversAllTransitions_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("hasPrev");
    QTest::addColumn<int>("prevValue");
    QTest::addColumn<WaveSegment>("expected");

    QTest::newRow("low, no previous cell")       << 0 << false << 0 << WaveSegment::Low;
    QTest::newRow("low, previous was low")       << 0 << true  << 0 << WaveSegment::Low;
    QTest::newRow("low, previous was high")      << 0 << true  << 1 << WaveSegment::Falling;
    QTest::newRow("high, no previous cell")      << 1 << false << 0 << WaveSegment::High;
    QTest::newRow("high, previous was high")     << 1 << true  << 1 << WaveSegment::High;
    QTest::newRow("high, previous was low")      << 1 << true  << 0 << WaveSegment::Rising;
}

void TestSignalDelegate::testSegmentForCoversAllTransitions()
{
    QFETCH(int, value);
    QFETCH(bool, hasPrev);
    QFETCH(int, prevValue);
    QFETCH(WaveSegment, expected);

    QCOMPARE(SignalDelegate::segmentFor(value, hasPrev, prevValue), expected);
}

void TestSignalDelegate::testPaintOutputRowUsesDifferentColorThanInputRow()
{
    // Row 0 is an input row, row 1 is an output row (setInputRows(1)); both hold the same
    // value pattern so the only difference the delegate can paint is the input/output color.
    SignalModel model(2, 2);
    model.setInputRows(1);
    model.setValue(0, 0, 1);
    model.setValue(0, 1, 1);
    model.setValue(1, 0, 1);
    model.setValue(1, 1, 1);

    SignalDelegate delegate;
    delegate.setPlotType(PlotType::Line);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 40, 30);

    const QImage inputImage = renderCell(delegate, option, model.index(0, 1));
    const QImage outputImage = renderCell(delegate, option, model.index(1, 1));

    QVERIFY2(inputImage != outputImage, "An output row must paint in a different color than an input row");
}
