// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simplewaveform.h"
#include "ui_simplewaveform.h"

#include "common.h"
#include "elementfactory.h"
#include "elementmapping.h"
#include "graphicelement.h"
#include "graphicelementinput.h"
#include "qneport.h"
#include "scene.h"
#include "settings.h"
#include "simulation.h"
#include "simulationblocker.h"
#include "workspace.h"

#include <QChartView>
#include <QClipboard>
#include <QDebug>
#include <QLineSeries>
#include <QMimeData>
#include <QValueAxis>
#include <bitset>
#include <cmath>

SimpleWaveform::SimpleWaveform(WorkSpace *workspace, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::SimpleWaveform)
    , m_chartView(new QChartView(&m_chart, this))
    , m_sortingMode(SortingMode::Increasing)
    , m_workspace(workspace)
{
    m_ui->setupUi(this);
    resize(800, 500);
    m_chart.legend()->setAlignment(Qt::AlignLeft);

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_ui->gridLayout->addWidget(m_chartView);
    setWindowTitle("Simple WaveForm - beWavedDolphin Beta");
    setWindowFlags(Qt::Window);
    setModal(true);
    restoreGeometry(Settings::value("SimpleWaveform/geometry").toByteArray());

    connect(m_ui->pushButtonCopy,        &QPushButton::clicked,  this, &SimpleWaveform::on_pushButtonCopy_clicked);
    connect(m_ui->radioButtonDecreasing, &QRadioButton::clicked, this, &SimpleWaveform::on_radioButtonDecreasing_clicked);
    connect(m_ui->radioButtonIncreasing, &QRadioButton::clicked, this, &SimpleWaveform::on_radioButtonIncreasing_clicked);
    connect(m_ui->radioButtonPosition,   &QRadioButton::clicked, this, &SimpleWaveform::on_radioButtonPosition_clicked);
}

SimpleWaveform::~SimpleWaveform()
{
    Settings::setValue("SimpleWaveform/geometry", saveGeometry());
    delete m_ui;
}

void SimpleWaveform::sortElements(QVector<GraphicElement *> &elements,
                                  QVector<GraphicElement *> &inputs,
                                  QVector<GraphicElement *> &outputs,
                                  SortingMode sorting)
{
    elements = ElementMapping::sortGraphicElements(elements);
    for (auto *elm : qAsConst(elements)) {
        if (!elm || elm->type() != GraphicElement::Type) {
            continue;
        }

        if (elm->elementGroup() == ElementGroup::Input) {
            inputs.append(elm);
        } else if (elm->elementGroup() == ElementGroup::Output) {
            outputs.append(elm);
        }
    }
    if (sorting == SortingMode::Position) {
        std::stable_sort(inputs.begin(), inputs.end(), [](const auto &elm1, const auto &elm2) {
            return elm1->pos().ry() < elm2->pos().ry();
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](const auto &elm1, const auto &elm2) {
            return elm1->pos().ry() < elm2->pos().ry();
        });
        std::stable_sort(inputs.begin(), inputs.end(), [](const auto &elm1, const auto &elm2) {
            return elm1->pos().rx() < elm2->pos().rx();
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](const auto &elm1, const auto &elm2) {
            return elm1->pos().rx() < elm2->pos().rx();
        });
    } else if (sorting == SortingMode::Increasing) {
        std::stable_sort(inputs.begin(), inputs.end(), [](const auto &elm1, const auto &elm2) {
            return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](const auto &elm1, const auto &elm2) {
            return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
        });
    } else { // if (sorting == SortingMode::Decreasing) {
        std::stable_sort(inputs.begin(), inputs.end(), [](const auto &elm1, const auto &elm2) {
            return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) > 0;
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](const auto &elm1, const auto &elm2) {
            return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) > 0;
        });
    }
}

bool SimpleWaveform::saveToTxt(QTextStream &outStream, WorkSpace *workspace)
{
    QVector<GraphicElement *> elements = workspace->scene()->elements();
    QVector<GraphicElement *> inputs;
    QVector<GraphicElement *> outputs;

    // Sorting elements according to the radion option. All elements initially in elements vector. Then, inputs and outputs are extracted from it.
    sortElements(elements, inputs, outputs, SortingMode::Increasing);
    if (elements.isEmpty() || inputs.isEmpty() || outputs.isEmpty()) {
        return false;
    }
    // Getting digital circuit simulator.
    auto *simulation = workspace->simulation();
    // Creating class to pause main window simulator while creating waveform.
    SimulationBlocker simulationBlocker(simulation);

    // Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore
    // it after simulation.
    QVector<Status> oldValues(inputs.size());
    for (int in = 0; in < inputs.size(); ++in) {
        oldValues[in] = inputs[in]->outputPort()->status();
    }
    // Computing the number of iterations based on the number of inputs.
    int numIter = static_cast<int>(pow(2, inputs.size()));
    // Getting the number of outputs. Warning: this will not work if any inout element type in created.
    int outputCount = 0;
    for (auto *out : qAsConst(outputs)) {
        outputCount += out->inputSize();
    }
    // Creating results vector containing the output resulting values.
    QVector<QVector<int>> results(outputCount, QVector<int>(numIter));
    for (int itr = 0; itr < numIter; ++itr) {
        // For each iteration, set a distinct value for the inputs. The set value corresponds to the bits from the number of the current iteration.
        std::bitset<std::numeric_limits<unsigned int>::digits> bs(itr);
        for (int in = 0; in < inputs.size(); ++in) {
            if (auto *input = qobject_cast<GraphicElementInput *>(inputs.at(in))) {
                input->setOn(bs[in]);
            }
        }
        // Updating the values of the circuit logic based on current input values.
        simulation->update();
        simulation->updateScene();
        // Setting the computed output values to the waveform results vector.
        int counter = 0;
        for (auto *output : qAsConst(outputs)) {
            const int inSize = output->inputSize();
            for (int port = inSize - 1; port >= 0; --port) {
                const int val = static_cast<int>(output->inputPort(port)->status());
                results[counter][itr] = val;
                counter++;
            }
        }
    }
    // Writing the input value of each iteration to the output stream.
    for (int in = 0; in < inputs.size(); ++in) {
        QString label = inputs[in]->label();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(inputs[in]->elementType());
        }
        for (int itr = 0; itr < numIter; ++itr) {
            std::bitset<std::numeric_limits<unsigned int>::digits> bs(itr);
            outStream << static_cast<int>(bs[in]);
        }
        outStream << " : \"" << label << "\"\n";
    }
    outStream << "\n";
    // Writing the output values at each iteration to the output stream.
    int counter = 0;
    for (auto *output : qAsConst(outputs)) {
        QString label = output->label();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }
        int inSize = output->inputSize();
        for (int port = inSize - 1; port >= 0; --port) {
            for (int itr = 0; itr < numIter; ++itr) {
                outStream << static_cast<int>(results[counter][itr]);
            }
            ++counter;
            outStream << " : \"" << label << "[" << port << "]\"\n";
        }
    }
    // Restoring the old values to the inputs, prior to simulaton.
    for (int in = 0; in < inputs.size(); ++in) {
        auto *input = qobject_cast<GraphicElementInput *>(inputs.at(in));
        input->setOn(static_cast<bool>(oldValues.at(in)));
    }
    // Resuming digital circuit main window after waveform simulation is finished.
    return true;
}

// Idea:
// 1) Split this function into parts. One to configure, one to load default or file values, one to simulate and one to show the result.
// 2) So, when opening the graphic simulator, there could be a button for the default simulation and another option to load a file (.dolphin,.csv).
// 3) To run by command line, the result could be saved to file.
void SimpleWaveform::showWaveform()
{
    qCDebug(zero) << tr("Getting sorting type.");
    if (Settings::contains("waveform/sortingType")) {
        m_sortingMode = static_cast<SortingMode>(Settings::value("waveform/sortingType").toInt());
    }
    switch (m_sortingMode) {
    case SortingMode::Decreasing: m_ui->radioButtonDecreasing->setChecked(true); break;
    case SortingMode::Increasing: m_ui->radioButtonIncreasing->setChecked(true); break;
    case SortingMode::Position:   m_ui->radioButtonPosition->setChecked(true); break;
    }
    int gap = 2;
    qCDebug(zero) << tr("Clear previous chart.");
    m_chart.removeAllSeries();
    qCDebug(zero) << tr("Getting digital circuit simulator.");
    auto *simulation = m_workspace->simulation();
    qCDebug(zero) << tr("Creating class to pause main window simulator while creating waveform.");
    SimulationBlocker simulationBlocker(simulation);
    QVector<GraphicElement *> elements = m_workspace->scene()->elements();
    QVector<GraphicElement *> inputs;
    QVector<GraphicElement *> outputs;
    qCDebug(zero) << tr("Sorting elements according to the radion option. All elements initially in elements vector. Then, inputs and outputs are extracted from it.");
    sortElements(elements, inputs, outputs, m_sortingMode);
    if (elements.isEmpty()) {
        throw Pandaception(tr("Could not find any port for the simulation"));
    }
    if (inputs.isEmpty()) {
        throw Pandaception(tr("Could not find any input for the simulation."));
    }
    if (outputs.isEmpty()) {
        throw Pandaception(tr("Could not find any output for the simulation."));
    }
    if (inputs.size() > 8) {
        throw Pandaception(tr("The simulation is limited to 8 inputs."));
    }
    QVector<QLineSeries *> inSeries;
    qCDebug(zero) << tr("Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. "
                        "Not saving memory states though...");
    qCDebug(zero) << tr("Also getting the name of the inputs. If no label is given, the element type is used as a name. "
                        "Bug here? What if there are 2 inputs without name or two identical labels?");
    QVector<Status> oldValues(inputs.size());
    inSeries.reserve(inputs.size());
    for (int in = 0; in < inputs.size(); ++in) {
        inSeries.append(new QLineSeries(this));
        QString label = inputs[in]->label();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(inputs[in]->elementType());
        }
        inSeries[in]->setName(label);
        oldValues[in] = inputs[in]->outputPort()->status();
    }
    QVector<QLineSeries *> outSeries;
    qCDebug(zero) << tr("Getting the name of the outputs. If no label is given, the element type is used as a name. "
                        "Bug here? What if there are 2 outputs without name or two identical labels?");
    for (auto *output : qAsConst(outputs)) {
        QString label = output->label();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }
        for (int port = 0; port < output->inputSize(); ++port) {
            outSeries.append(new QLineSeries(this));
            if (output->inputSize() > 1) {
                outSeries.last()->setName(label + "_" + QString::number(port));
            } else {
                outSeries.last()->setName(label);
            }
        }
    }
    qCDebug(zero) << inSeries.size() << tr("inputs.");
    qCDebug(zero) << outSeries.size() << tr("outputs.");
    qCDebug(zero) << tr("Computing number of iterations based on the number of inputs.");
    int numIter = static_cast<int>(pow(2, inSeries.size()));
    qCDebug(zero) << tr("Num iter = ") << numIter;
    // gap += outputs.size() % 2;
    qCDebug(zero) << tr("Running simulation.");
    for (int itr = 0; itr < numIter; ++itr) {
        qCDebug(three) << tr("For each iteration, set a distinct value for the inputs. The value is the bit values corresponding to the number of the current iteration.");
        std::bitset<std::numeric_limits<unsigned int>::digits> bs(itr);
        qCDebug(three) << tr("itr:") << itr;
        for (int in = 0; in < inputs.size(); ++in) {
            float val = static_cast<float>(bs[in]);
            qobject_cast<GraphicElementInput *>(inputs[in])->setOn(!qFuzzyIsNull(val));
            float offset = static_cast<float>((inSeries.size() - in - 1 + outSeries.size()) * 2 + gap + 0.5);
            inSeries[in]->append(itr, static_cast<qreal>(offset + val));
            inSeries[in]->append(itr + 1, static_cast<qreal>(offset + val));
        }
        qCDebug(three) << tr("Updating the values of the circuit logic based on current input values.");
        simulation->update();
        simulation->updateScene();
        qCDebug(three) << tr("Setting the computed output values to the waveform results.");
        int counter = 0;
        for (auto *output : qAsConst(outputs)) {
            int inSize = output->inputSize();
            for (int port = inSize - 1; port >= 0; --port) {
                float val = static_cast<float>(output->inputPort(port)->status() == Status::Active);
                float offset = static_cast<float>((outSeries.size() - counter - 1) * 2 + 0.5);
                outSeries.at(counter)->append(itr, static_cast<qreal>(offset + val));
                outSeries.at(counter)->append(itr + 1, static_cast<qreal>(offset + val));
                // cout << counter << " " << out;
                counter++;
            }
        }
    }
    qCDebug(three) << tr("Inserting input series to the chart.");
    for (auto *in : qAsConst(inSeries)) {
        m_chart.addSeries(in);
    }
    qCDebug(three) << tr("Inserting output series to the chart.");
    for (auto *out : qAsConst(outSeries)) {
        m_chart.addSeries(out);
    }
    qCDebug(three) << tr("Setting graphic axes.");
    m_chart.createDefaultAxes();

    // chart.axisY()->hide();
    qCDebug(zero) << tr("Setting range and names to x, y axis.");
    const auto horizontalAxes = m_chart.axes(Qt::Horizontal);
    const auto verticalAxes = m_chart.axes(Qt::Vertical);

    auto *axisX = qobject_cast<QValueAxis *>(horizontalAxes.last());
    axisX->setRange(0, numIter);
    axisX->setTickCount(numIter + 1);
    axisX->setLabelFormat("%i");
    auto *axixY = qobject_cast<QValueAxis *>(verticalAxes.last());
    // ay->setShadesBrush(QBrush(Qt::lightGray));

    qCDebug(zero) << tr("Setting graphics waveform color.");
    axixY->setShadesColor(QColor(0, 0, 0, 8));
    axixY->setShadesPen(QPen(QColor(0, 0, 0, 0)));
    axixY->setShadesVisible(true);
    axixY->setGridLineVisible(false);
    axixY->setTickCount(inSeries.size() + outSeries.size() + gap / 2 + 1);
    axixY->setRange(0, inSeries.size() * 2 + outSeries.size() * 2 + gap);
    axixY->setGridLineColor(Qt::transparent);
    axixY->setLabelsVisible(false);
    // ay->hide();
    qCDebug(zero) << tr("Executing QDialog. Opens window to the user.");
    exec();
    qCDebug(zero) << tr("Restoring the old values to the inputs, prior to simulaton.");
    for (int in = 0; in < inputs.size(); ++in) {
        auto *input = qobject_cast<GraphicElementInput *>(inputs.at(in));
        input->setOn(static_cast<bool>(oldValues.at(in)));
    }
    qCDebug(zero) << tr("Resuming digital circuit main window after waveform simulation is finished.");
}

void SimpleWaveform::on_radioButtonPosition_clicked()
{
    m_sortingMode = SortingMode::Position;
    Settings::setValue("waveform/sortingType", static_cast<int>(m_sortingMode));
    showWaveform();
}

void SimpleWaveform::on_radioButtonIncreasing_clicked()
{
    m_sortingMode = SortingMode::Increasing;
    Settings::setValue("waveform/sortingType", static_cast<int>(m_sortingMode));
    showWaveform();
}

void SimpleWaveform::on_radioButtonDecreasing_clicked()
{
    m_sortingMode = SortingMode::Decreasing;
    Settings::setValue("waveform/sortingType", static_cast<int>(m_sortingMode));
    showWaveform();
}

void SimpleWaveform::on_pushButtonCopy_clicked()
{
    QSize size = m_chart.size().toSize();
    QPixmap pixmap(size);
    pixmap.fill(Qt::white);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_chart.paint(&painter, nullptr, nullptr); /* This gives 0 items in 1 group */
    m_chartView->render(&painter); /* m_view has app->chart() in it, and this one gives right image */
    qCDebug(zero) << tr("Copied.");
    painter.end();
    auto *mimeData = new QMimeData();
    mimeData->setImageData(pixmap);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}
