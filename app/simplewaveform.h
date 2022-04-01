/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "editor.h"

#include <QChart>
#include <QChartView>
#include <QDialog>
#include <QTextStream>

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
    explicit SimpleWaveform(Editor *editor, QWidget *parent = nullptr);
    ~SimpleWaveform() override;

    static bool saveToTxt(QTextStream &outStream, Editor *editor);
    static void sortElements(QVector<GraphicElement *> &elements, QVector<GraphicElement *> &inputs, QVector<GraphicElement *> &outputs, SortingMode sorting);

    void showWaveform();

private:
    void on_pushButton_Copy_clicked();
    void on_radioButton_Decreasing_clicked();
    void on_radioButton_Increasing_clicked();
    void on_radioButton_Position_clicked();

    Ui::SimpleWaveform *m_ui;
    Editor *m_editor;
    QChart m_chart;
    QChartView *m_chartView;
    SortingMode m_sortingMode;
};

