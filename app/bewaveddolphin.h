// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicsview.h"
#include "scene.h"

#include <QFileInfo>
#include <QItemDelegate>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>
#include <optional>

class GraphicsView;
class MainWindow;
class QItemSelection;
class QSaveFile;

namespace Ui
{
class BewavedDolphin;
}

enum class PlotType { Number, Line };

class SignalModel : public QStandardItemModel
{
    Q_OBJECT

public:
    SignalModel(const int inputs, const int rows, const int columns, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    const int m_inputCount;
};

class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SignalDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class DolphinGraphicsView : public GraphicsView
{
    Q_OBJECT

public:
    explicit DolphinGraphicsView(QWidget *parent = nullptr);
    bool canZoomOut() const;

    inline static bool canNavigateWithArrowKeys = true;

signals:
    void scaleIn();
    void scaleOut();

protected:
    void wheelEvent(QWheelEvent *event) override;
};

class BewavedDolphin : public QMainWindow
{
    Q_OBJECT

public:
    explicit BewavedDolphin(Scene *scene, const bool askConnection = true, MainWindow *parent = nullptr);
    ~BewavedDolphin() override;

    void createWaveform();
    void createWaveform(const QString &fileName);
    void print();
    void saveToTxt(QTextStream &stream);
    void show();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Q_DISABLE_COPY(BewavedDolphin)

    /**
     * creates an image composed of eight parts, following the pattern given by the vector of booleans.
     * @param waveparts contains the values useds to create the final waveform.
     * @param isInput is used to knows if the color will be blue (if is True) or green (if is False).
     * @param lastValue is used to knows the last state to create the first state of the current waveform.
     * @return a Pixmap containing the waveform image 64x38 pixels.
    */
    QPixmap composeWaveParts(const QVector<bool> waveparts, std::optional<bool> previousWaveEnd, const bool isInput = true);

    /**
     * This function converts a boolean vector into a string of hexadecimal values.
     * @param binaryVector is a boolean vector with eight positions.
     * @return returns a string with the corresponding hexadecimal of the binaryVector.
    */
    std::string convertBinaryToHex(const QVector<bool> binaryVector) const;

    bool checkSave();
    int sectionFirstColumn(const QItemSelection &ranges);
    int sectionFirstRow(const QItemSelection &ranges);
    void associateToWiredPanda(const QString &fileName);
    void copy(const QItemSelection &ranges, QDataStream &stream);
    void createElement(const int row, const int col, const int value, const bool isInput = true, const bool changeNext = true);
    void createOneElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);

    /**
     * Plot the composedWaveform in the row and column correctly.
     * @param row is the number of the line that you want to plot.
     * @param col is the number of the column that you want to plot.
     * @param composedWaveform is the pixmap that will be ploted.
     * @param isInput is True if is about an input waveform or False if you want an output waveform.
     * @param changeNext is True if you want to change next value in the row, and False if you don't.
    */
    void createTemporalSimulationElement(const int row, const int col, QPixmap composedWaveForm, const std::string hex, const bool isInput = true, const bool changeNext = true);

    void createZeroElement(const int row, const int col, const bool isInput = true, const bool changeNext = true);

    /**
     * Calls the composeWaveParts to create the composed waveform, then calls the CreateTemporalSimulationElement to plot the pixmap corretly.
     * @param row is the number of the line that you want to plot.
     * @param col is the number of the column that you want to plot.
     * @param output contains the values useds to create the final waveform in composeWaveParts().
     * @param lastValue is used to knows the last state to create the first state of the current waveform.
     * @param isInput is True if is about an input waveform or False if you want an output waveform.
     * @param changeNext is True if you want to change next value in the row, and False if you don't.
    */
    void createComposedWaveFormElement(const int row, const int column, QVector<bool> output, std::optional<bool> previousWaveEnd, const bool isInput = true, const bool changeNext = true);

    void cut(const QItemSelection &ranges, QDataStream &stream);
    void load(QDataStream &stream);
    void load(QFile &file);
    void load(const QString &fileName);
    void loadElements();
    void loadFromTerminal();
    void loadNewTable();
    void loadPixmaps();
    void loadSignals(QStringList &inputLabels, QStringList &outputLabels);
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionClear_triggered();
    void on_actionCombinational_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionExit_triggered();
    void on_actionExportToPdf_triggered();
    void on_actionExportToPng_triggered();
    void on_actionInvert_triggered();
    void on_actionLoad_triggered();
    void on_actionPaste_triggered();
    void on_actionResetZoom_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSave_triggered();
    void on_actionSetClockWave_triggered();
    void on_actionSetLength_triggered();
    void on_actionSetTo0_triggered();
    void on_actionSetTo1_triggered();
    void on_actionShowNumbers_triggered();
    void on_actionShowWaveforms_triggered();
    void on_actionTemporalSimulation_toggled(const bool checked);
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_tableView_cellDoubleClicked();
    void on_tableView_selectionChanged();
    void paste(const QItemSelection &ranges, QDataStream &stream);
    void prepare(const QString &fileName = {});
    void resizeScene();
    void restoreInputs();
    void run();
    void run2();
    void save(QDataStream &stream);
    void save(QSaveFile &file);
    void save(const QString &fileName);
    void setLength(const int simLength, const bool runSimulation);
    void zoomChanged();

    Ui::BewavedDolphin *m_ui;
    MainWindow *m_mainWindow = nullptr;
    PlotType m_type = PlotType::Line;
    QFileInfo m_currentFile;
    QGraphicsScene *m_scene = new QGraphicsScene(this);
    QPixmap m_fallingBlue;
    QPixmap m_fallingGreen;
    QPixmap m_highBlue;
    QPixmap m_highGreen;
    QPixmap m_lowBlue;
    QPixmap m_lowGreen;
    QPixmap m_risingBlue;
    QPixmap m_risingGreen;
    QPixmap m_smallFallingBlue;
    QPixmap m_smallFallingGreen;
    QPixmap m_smallHighBlue;
    QPixmap m_smallHighGreen;
    QPixmap m_smallLowBlue;
    QPixmap m_smallLowGreen;
    QPixmap m_smallRisingBlue;
    QPixmap m_smallRisingGreen;
    QStandardItemModel *m_model = nullptr;
    QTableView *m_signalTableView = new QTableView();
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<Status> m_oldInputValues;
    QVector<QVector<bool>> output_values;
    Scene *m_externalScene = nullptr;
    Simulation *m_simulation = nullptr;
    bool m_edited = false;
    bool m_isTemporalSimulation = false;
    const bool m_askConnection;
    const double m_scaleFactor = 0.8;
    double m_scale = 1.25;
    int m_clockPeriod = 0;
    int m_inputPorts = 0;
    int m_length = 32;
};
