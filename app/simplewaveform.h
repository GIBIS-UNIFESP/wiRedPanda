/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SIMPLEWAVEFORM_H
#define SIMPLEWAVEFORM_H

#include "editor.h"

#include <QChart>
#include <QChartView>
#include <QDialog>
#include <QTextStream>

namespace Ui
{
class SimpleWaveform;
}

class SimpleWaveform : public QDialog
{
    Q_OBJECT
    enum class SortingMode : uint_fast8_t { INCREASING, DECREASING, POSITION };

public:
    explicit SimpleWaveform(Editor *editor, QWidget *parent = nullptr);
    ~SimpleWaveform() override;

    void showWaveform();
    static void sortElements(QVector<GraphicElement *> &elements, QVector<GraphicElement *> &inputs, QVector<GraphicElement *> &outputs, SortingMode sorting);

    static bool saveToTxt(QTextStream &outStream, Editor *editor);

private slots:
    void on_radioButton_Position_clicked();
    void on_radioButton_Increasing_clicked();
    void on_radioButton_Decreasing_clicked();
    void on_pushButton_Copy_clicked();

private:
    Ui::SimpleWaveform *m_ui;
    QtCharts::QChart m_chart;
    QtCharts::QChartView *m_chartView;
    Editor *m_editor;

    SortingMode m_sortingMode;
};

#endif /* SIMPLEWAVEFORM_H */
