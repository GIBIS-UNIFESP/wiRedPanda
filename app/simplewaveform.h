/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "workspace.h"

#include <QChartView>
#include <QDialog>

class QTextStream;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace QtCharts;
#endif

namespace Ui
{
class SimpleWaveform;
}

class SimpleWaveform : public QDialog
{
    Q_OBJECT

    enum class SortingMode : uint_fast8_t { Increasing, Decreasing, Position };

public:
    explicit SimpleWaveform(WorkSpace *workspace, QWidget *parent = nullptr);
    ~SimpleWaveform() override;

    static bool saveToTxt(QTextStream &outStream, WorkSpace *workspace);
    static void sortElements(QVector<GraphicElement *> &elements, QVector<GraphicElement *> &inputs, QVector<GraphicElement *> &outputs, SortingMode sorting);

    void showWaveform();

private:
    void on_pushButtonCopy_clicked();
    void on_radioButtonDecreasing_clicked();
    void on_radioButtonIncreasing_clicked();
    void on_radioButtonPosition_clicked();

    Ui::SimpleWaveform *m_ui;
    QChart m_chart;
    QChartView *m_chartView;
    SortingMode m_sortingMode;
    WorkSpace *m_workspace;
};
