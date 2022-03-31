// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simplewaveform.h"
#include "ui_simplewaveform.h"

#include "common.h"
#include "elementfactory.h"
#include "elementmapping.h"
#include "graphicelement.h"
#include "input.h"
#include "qneport.h"
#include "scstop.h"
#include "settings.h"
#include "simulationcontroller.h"

#include <QChartView>
#include <QClipboard>
#include <QDebug>
#include <QLineSeries>
#include <QMessageBox>
#include <QMimeData>
#include <QValueAxis>
#include <bitset>
#include <cmath>

SimpleWaveform::SimpleWaveform(Editor *editor, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::SimpleWaveform)
    , m_editor(editor)
{
    m_ui->setupUi(this);
    resize(800, 500);
    m_chart.legend()->setAlignment(Qt::AlignLeft);

    m_chartView = new QChartView(&m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_ui->gridLayout->addWidget(m_chartView);
    setWindowTitle("Simple WaveForm - beWavedDolphin Beta");
    setWindowFlags(Qt::Window);
    setModal(true);
<<<<<<< HEAD:app/SimpleWaveForm.cpp
    m_sortingMode = SortingMode::INCREASING;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("SimpleWaveform");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
    
    connect( m_ui->pushButton, &QPushButton::clicked, this, &SimpleWaveform::pushButtonCopyClicked);
    connect( m_ui->radioButton_Decreasing, &QPushButton::clicked, this, &SimpleWaveform::radioButtonDecreasingClicked);
    connect( m_ui->radioButton_Position, &QPushButton::clicked, this, &SimpleWaveform::radioButtonPositionClicked);
    connect( m_ui->radioButton_Increasing, &QPushButton::clicked, this, &SimpleWaveform::radioButtonIncreasingClicked);
    
    
    
=======
    m_sortingMode = SortingMode::Increasing;
    Settings::beginGroup("SimpleWaveform");
    restoreGeometry(Settings::value("geometry").toByteArray());
    Settings::endGroup();
>>>>>>> master:app/simplewaveform.cpp
}

SimpleWaveform::~SimpleWaveform()
{
    Settings::beginGroup("SimpleWaveform");
    Settings::setValue("geometry", saveGeometry());
    Settings::endGroup();
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
            return QString::compare(elm1->getLabel(), elm2->getLabel(), Qt::CaseInsensitive) < 0;
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](const auto &elm1, const auto &elm2) {
            return QString::compare(elm1->getLabel(), elm2->getLabel(), Qt::CaseInsensitive) < 0;
        });
    } else { // if (sorting == SortingMode::Decreasing) {
        std::stable_sort(inputs.begin(), inputs.end(), [](const auto &elm1, const auto &elm2) {
            return QString::compare(elm1->getLabel(), elm2->getLabel(), Qt::CaseInsensitive) > 0;
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](const auto &elm1, const auto &elm2) {
            return QString::compare(elm1->getLabel(), elm2->getLabel(), Qt::CaseInsensitive) > 0;
        });
    }
}

bool SimpleWaveform::saveToTxt(QTextStream &outStream, Editor *editor)
{
    QVector<GraphicElement *> elements = editor->getScene()->getElements();
    QVector<GraphicElement *> inputs;
    QVector<GraphicElement *> outputs;

    // Sorting elements according to the radion option. All elements initially in elements vector. Then, inputs and outputs are extracted from it.
    sortElements(elements, inputs, outputs, SortingMode::Increasing);
    if (elements.isEmpty() || inputs.isEmpty() || outputs.isEmpty()) {
        return false;
    }
    // Getting digital circuit simulator.
    SimulationController *sc = editor->getSimulationController();
    // Creating class to pause main window simulator while creating waveform.
    SCStop scst(sc);

    // Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore
    // it after simulation.
    QVector<char> oldValues(inputs.size());
    for (int in = 0; in < inputs.size(); ++in) {
        oldValues[in] = inputs[in]->output()->value();
    }
    // Computing the number of iterations based on the number of inputs.
    int num_iter = pow(2, inputs.size());
    // Getting the number of outputs. Warning: this will not work if any inout element type in created.
    int outputCount = 0;
    for (auto *out : qAsConst(outputs)) {
        outputCount += out->inputSize();
    }
    // Creating results vector containing the output resulting values.
    QVector<QVector<uchar>> results(outputCount, QVector<uchar>(num_iter));
    for (int itr = 0; itr < num_iter; ++itr) {
        // For each iteration, set a distinct value for the inputs. The set value corresponds to the bits from the number of the current iteration.
        std::bitset<std::numeric_limits<unsigned int>::digits> bs(itr);
        for (int in = 0; in < inputs.size(); ++in) {
            uchar val = bs[in];
            dynamic_cast<Input *>(inputs[in])->setOn(val);
        }
        // Updating the values of the circuit logic based on current input values.
        sc->update();
        sc->updateAll();
        // Setting the computed output values to the waveform results vector.
        int counter = 0;
        for (auto *output : outputs) {
            int inSz = output->inputSize();
            for (int port = inSz - 1; port >= 0; --port) {
                uchar val = output->input(port)->value();
                results[counter][itr] = val;
                counter++;
            }
        }
    }
    // Writing the input value of each iteration to the output stream.
    for (int in = 0; in < inputs.size(); ++in) {
        QString label = inputs[in]->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(inputs[in]->elementType());
        }
        for (int itr = 0; itr < num_iter; ++itr) {
            std::bitset<std::numeric_limits<unsigned int>::digits> bs(itr);
            outStream << static_cast<int>(bs[in]);
        }
        outStream << " : \"" << label << "\"\n";
    }
    outStream << "\n";
    // Writing the output values at each iteration to the output stream.
    int counter = 0;
    for (auto *output : outputs) {
        QString label = output->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }
        int inSz = output->inputSize();
        for (int port = inSz - 1; port >= 0; --port) {
            for (int itr = 0; itr < num_iter; ++itr) {
                outStream << static_cast<int>(results[counter][itr]);
            }
            ++counter;
            outStream << " : \"" << label << "[" << port << "]\"\n";
        }
    }
    // Restoring the old values to the inputs, prior to simulaton.
    for (int in = 0; in < inputs.size(); ++in) {
        dynamic_cast<Input *>(inputs[in])->setOn(oldValues[in]);
    }
    // Resuming digital circuit main window after waveform simulation is finished.
    scst.release();
    return true;
}

// Ideia: 1) Dividir essa função em partes. Uma para configurar, uma para carregar valores padrão ou de arquivo, uma para simular e uma para mostrar o
// resultado. 2) Assim, ao abrir o simulador gráfico, poderia ter botão para simulação padrão e outra opção para carregar arquivo (.dolphin,.csv). 3) Para rodar
// por linha de comando, o resultado poderia ser salvo em arquivo.
void SimpleWaveform::showWaveform()
{
    Settings::beginGroup("waveform");
    COMMENT("Getting sorting type.", 0);
    if (Settings::contains("sortingType")) {
        m_sortingMode = static_cast<SortingMode>(Settings::value("sortingType").toInt());
    }
    Settings::endGroup();
    switch (m_sortingMode) {
    case SortingMode::Decreasing: m_ui->radioButton_Decreasing->setChecked(true); break;
    case SortingMode::Increasing: m_ui->radioButton_Increasing->setChecked(true); break;
    case SortingMode::Position:   m_ui->radioButton_Position->setChecked(true); break;
    }
    int gap = 2;
    COMMENT("Clear previous chart.", 0);
    m_chart.removeAllSeries();
    COMMENT("Getting digital circuit simulator.", 0);
    SimulationController *sc = m_editor->getSimulationController();
    COMMENT("Creating class to pause main window simulator while creating waveform.", 0);
    SCStop scst(sc);
    QVector<GraphicElement *> elements = m_editor->getScene()->getElements();
    QVector<GraphicElement *> inputs;
    QVector<GraphicElement *> outputs;
    COMMENT("Sorting elements according to the radion option. All elements initially in elements vector. Then, inputs and outputs are extracted from it.", 0);
    sortElements(elements, inputs, outputs, m_sortingMode);
    if (elements.isEmpty()) {
        QMessageBox::warning(parentWidget(), tr("Error"), tr("Could not find any port for the simulation"));
        return;
    }
    if (inputs.isEmpty()) {
        QMessageBox::warning(parentWidget(), tr("Error"), tr("Could not find any input for the simulation."));
        return;
    }
    if (outputs.isEmpty()) {
        QMessageBox::warning(parentWidget(), tr("Error"), tr("Could not find any output for the simulation."));
        return;
    }
    if (inputs.size() > 8) {
        QMessageBox::warning(parentWidget(), tr("Error"), tr("The simulation is limited to 8 inputs."));
        return;
    }
    QVector<QLineSeries *> in_series;
    COMMENT(
        "Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. Not saving "
        "memory states though...",
        0);
    COMMENT(
        "Also getting the name of the inputs. If no label is given, the element type is used as a name. Bug here? What if there are 2 inputs without name or "
        "two identical labels?",
        0);
    QVector<char> oldValues(inputs.size());
    for (int in = 0; in < inputs.size(); ++in) {
        in_series.append(new QLineSeries(this));
        QString label = inputs[in]->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(inputs[in]->elementType());
        }
        in_series[in]->setName(label);
        oldValues[in] = inputs[in]->output()->value();
    }
    QVector<QLineSeries *> out_series;
    COMMENT(
        "Getting the name of the outputs. If no label is given, the element type is used as a name. Bug here? What if there are 2 outputs without name or two "
        "identical labels?",
        0);
    for (auto *output : outputs) {
        QString label = output->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }
        for (int port = 0; port < output->inputSize(); ++port) {
            out_series.append(new QLineSeries(this));
            if (output->inputSize() > 1) {
                out_series.last()->setName(QString("%1_%2").arg(label).arg(port));
            } else {
                out_series.last()->setName(label);
            }
        }
    }
    COMMENT(in_series.size() << " inputs", 0);
    COMMENT(out_series.size() << " outputs", 0);
    COMMENT("Computing number of iterations based on the number of inputs.", 0);
    int num_iter = pow(2, in_series.size());
    COMMENT("Num iter = " << num_iter, 0);
    // gap += outputs.size() % 2;
    COMMENT("Running simulation.", 0);
    for (int itr = 0; itr < num_iter; ++itr) {
        COMMENT("For each iteration, set a distinct value for the inputs. The value is the bit values corresponding to the number of the current iteration.", 3);
        std::bitset<std::numeric_limits<unsigned int>::digits> bs(itr);
        COMMENT("itr: " << itr, 3);
        for (int in = 0; in < inputs.size(); ++in) {
            float val = bs[in];
            dynamic_cast<Input *>(inputs[in])->setOn(!qFuzzyIsNull(val));
            float offset = (in_series.size() - in - 1 + out_series.size()) * 2 + gap + 0.5;
            in_series[in]->append(itr, static_cast<qreal>(offset + val));
            in_series[in]->append(itr + 1, static_cast<qreal>(offset + val));
        }
        COMMENT("Updating the values of the circuit logic based on current input values.", 3);
        sc->update();
        sc->updateAll();
        COMMENT("Setting the computed output values to the waveform results.", 3);
        int counter = 0;
        for (auto *output : outputs) {
            int inSz = output->inputSize();
            for (int port = inSz - 1; port >= 0; --port) {
                float val = output->input(port)->value() > 0;
                float offset = (out_series.size() - counter - 1) * 2 + 0.5;
                out_series[counter]->append(itr, static_cast<qreal>(offset + val));
                out_series[counter]->append(itr + 1, static_cast<qreal>(offset + val));
                // cout << counter << " " << out;
                counter++;
            }
        }
    }
    COMMENT("Inserting input series to the chart.", 3);
    for (auto *in : qAsConst(in_series)) {
        m_chart.addSeries(in);
    }
    COMMENT("Inserting output series to the chart.", 3);
    for (auto *out : qAsConst(out_series)) {
        m_chart.addSeries(out);
    }
    COMMENT("Setting graphic axes.", 3);
    m_chart.createDefaultAxes();

    // chart.axisY()->hide();
    COMMENT("Setting range and names to x, y axis.", 0);
    const auto horizontal_axe = m_chart.axes(Qt::Horizontal);
    const auto vertical_axe = m_chart.axes(Qt::Vertical);

    auto *ax = qobject_cast<QValueAxis *>(horizontal_axe.back());
    ax->setRange(0, num_iter);
    ax->setTickCount(num_iter + 1);
    ax->setLabelFormat(QString("%i"));
    auto *ay = qobject_cast<QValueAxis *>(vertical_axe.back());
    // ay->setShadesBrush(QBrush(Qt::lightGray));

    COMMENT("Setting graphics waveform color.", 0);
    ay->setShadesColor(QColor(0, 0, 0, 8));
    ay->setShadesPen(QPen(QColor(0, 0, 0, 0)));
    ay->setShadesVisible(true);
    ay->setGridLineVisible(false);
    ay->setTickCount((in_series.size() + out_series.size() + gap / 2 + 1));
    ay->setRange(0, in_series.size() * 2 + out_series.size() * 2 + gap);
    ay->setGridLineColor(Qt::transparent);
    ay->setLabelsVisible(false);
    // ay->hide();
    COMMENT("Executing QDialog. Opens window to the user.", 0);
    exec();
    COMMENT("Restoring the old values to the inputs, prior to simulaton.", 0);
    for (int in = 0; in < inputs.size(); ++in) {
        dynamic_cast<Input *>(inputs[in])->setOn(oldValues[in]);
    }
    COMMENT("Resuming digital circuit main window after waveform simulation is finished.", 0);
    scst.release();
}

void SimpleWaveform::radioButtonPositionClicked()
{
    Settings::beginGroup("waveform");
    m_sortingMode = SortingMode::Position;
    Settings::setValue("sortingType", static_cast<int>(m_sortingMode));
    Settings::endGroup();
    showWaveform();
}

void SimpleWaveform::radioButtonIncreasingClicked()
{
    Settings::beginGroup("waveform");
    m_sortingMode = SortingMode::Increasing;
    Settings::setValue("sortingType", static_cast<int>(m_sortingMode));
    Settings::endGroup();
    showWaveform();
}

void SimpleWaveform::radioButtonDecreasingClicked()
{
    Settings::beginGroup("waveform");
    m_sortingMode = SortingMode::Decreasing;
    Settings::setValue("sortingType", static_cast<int>(m_sortingMode));
    Settings::endGroup();
    showWaveform();
}

void SimpleWaveform::pushButtonCopyClicked()
{
    QSize s = m_chart.size().toSize();
    QPixmap p(s);
    p.fill(Qt::white);
    QPainter painter;
    painter.begin(&p);
    painter.setRenderHint(QPainter::Antialiasing);
    m_chart.paint(&painter, nullptr, nullptr); /* This gives 0 items in 1 group */
    m_chartView->render(&painter); /* m_view has app->chart() in it, and this one gives right image */
    qDebug() << "Copied";
    painter.end();
    auto *d = new QMimeData();
    d->setImageData(p);
    QApplication::clipboard()->setMimeData(d, QClipboard::Clipboard);
}
