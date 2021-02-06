// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simplewaveform.h"

#include <bitset>
#include <cmath>

#include <QChartView>
#include <QClipboard>
#include <QDebug>
#include <QLineSeries>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QValueAxis>

#include "elementfactory.h"
#include "elementmapping.h"
#include "graphicelement.h"
#include "qneport.h"
#include "scstop.h"
#include "simulationcontroller.h"
#include "ui_simplewaveform.h"

#include "input.h"

using namespace QtCharts;

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
    setWindowTitle("Simple WaveForm - WaveDolphin Beta");
    setWindowFlags(Qt::Window);
    setModal(true);
    m_sortingMode = SortingMode::INCREASING;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("SimpleWaveform");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
}

SimpleWaveform::~SimpleWaveform()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("SimpleWaveform");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
    delete m_ui;
}

void SimpleWaveform::sortElements(QVector<GraphicElement *> &elements,
                                  QVector<GraphicElement *> &inputs,
                                  QVector<GraphicElement *> &outputs,
                                  SortingMode sorting)
{
    elements = ElementMapping::sortGraphicElements(elements);
    for (GraphicElement *elm : qAsConst(elements)) {
        if (elm && (elm->type() == GraphicElement::Type)) {
            if (elm->elementGroup() == ElementGroup::INPUT) {
                inputs.append(elm);
            } else if (elm->elementGroup() == ElementGroup::OUTPUT) {
                outputs.append(elm);
            }
        }
    }
    if (sorting == SortingMode::POSITION) {
        std::stable_sort(inputs.begin(), inputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return elm1->pos().ry() < elm2->pos().ry();
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return elm1->pos().ry() < elm2->pos().ry();
        });
        std::stable_sort(inputs.begin(), inputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return elm1->pos().rx() < elm2->pos().rx();
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return elm1->pos().rx() < elm2->pos().rx();
        });
    } else if (sorting == SortingMode::INCREASING) {
        std::stable_sort(inputs.begin(), inputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return QString::compare(elm1->getLabel().toUtf8(), elm2->getLabel().toUtf8(), Qt::CaseInsensitive) <= 0;
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return QString::compare(elm1->getLabel().toUtf8(), elm2->getLabel().toUtf8(), Qt::CaseInsensitive) <= 0;
        });
    } else { // if( sorting == SortingMode::DECREASING ) {
        std::stable_sort(inputs.begin(), inputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return QString::compare(elm1->getLabel().toUtf8(), elm2->getLabel().toUtf8(), Qt::CaseInsensitive) >= 0;
        });
        std::stable_sort(outputs.begin(), outputs.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
            return QString::compare(elm1->getLabel().toUtf8(), elm2->getLabel().toUtf8(), Qt::CaseInsensitive) >= 0;
        });
    }
}

bool SimpleWaveform::saveToTxt(QTextStream &outStream, Editor *editor)
{
    QVector<GraphicElement *> elements = editor->getScene()->getElements();
    QVector<GraphicElement *> inputs;
    QVector<GraphicElement *> outputs;

    // Sorting elements according to the radion option. All elements initially in elements vector. Then, inputs and outputs are extracted from it.
    sortElements(elements, inputs, outputs, SortingMode::INCREASING);
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
    for (GraphicElement *out : qAsConst(outputs)) {
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
        for (int out = 0; out < outputs.size(); ++out) {
            int inSz = outputs[out]->inputSize();
            for (int port = inSz - 1; port >= 0; --port) {
                uchar val = outputs[out]->input(port)->value();
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
    for (int out = 0; out < outputs.size(); ++out) {
        QString label = outputs[out]->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(outputs[out]->elementType());
        }
        int inSz = outputs[out]->inputSize();
        for (int port = inSz - 1; port >= 0; --port) {
            for (int itr = 0; itr < num_iter; ++itr) {
                outStream << static_cast<int>(results[counter][itr]);
            }
            counter += 1;
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
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("waveform");
    qDebug() << "Getting sorting type.";
    if (settings.contains("sortingType")) {
        m_sortingMode = static_cast<SortingMode>(settings.value("sortingType").toInt());
    }
    settings.endGroup();
    switch (m_sortingMode) {
    case SortingMode::DECREASING:
        m_ui->radioButton_Decreasing->setChecked(true);
        break;
    case SortingMode::INCREASING:
        m_ui->radioButton_Increasing->setChecked(true);
        break;
    case SortingMode::POSITION:
        m_ui->radioButton_Position->setChecked(true);
        break;
    }
    int gap = 2;
    qDebug() << "Clear previous chart.";
    m_chart.removeAllSeries();
    qDebug() << "Getting digital circuit simulator.";
    SimulationController *sc = m_editor->getSimulationController();
    qDebug() << "Creating class to pause main window simulator while creating waveform.";
    SCStop scst(sc);
    QVector<GraphicElement *> elements = m_editor->getScene()->getElements();
    QVector<GraphicElement *> inputs;
    QVector<GraphicElement *> outputs;
    qDebug() << "Sorting elements according to the radion option. All elements initially in elements vector. Then, inputs and outputs are extracted from it.";
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
    qDebug() << "Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. Not saving memory states though...";
    qDebug() << "Also getting the name of the inputs. If no label is given, the element type is used as a name. Bug here? What if there are 2 inputs without name or two identical labels?";
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
    qDebug() << "Getting the name of the outputs. If no label is given, the element type is used as a name. Bug here? What if there are 2 outputs without name or two identical labels?";
    for (int out = 0; out < outputs.size(); ++out) {
        QString label = outputs[out]->getLabel();
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(outputs[out]->elementType());
        }
        for (int port = 0; port < outputs[out]->inputSize(); ++port) {
            out_series.append(new QLineSeries(this));
            if (outputs[out]->inputSize() > 1) {
                out_series.last()->setName(QString("%1_%2").arg(label).arg(port));
            } else {
                out_series.last()->setName(label);
            }
        }
    }
    qDebug() << in_series.size() << " inputs";
    qDebug() << out_series.size() << " outputs";
    qDebug() << "Computing number of iterations based on the number of inputs.";
    int num_iter = pow(2, in_series.size());
    qDebug() << "Num iter = " << num_iter;
    /*  gap += outputs.size( ) % 2; */
    qDebug() << "Running simulation.";
    for (int itr = 0; itr < num_iter; ++itr) {
        qDebug() << "For each iteration, set a distinct value for the inputs. The value is the bit values corresponding to the number of the current iteration.";
        std::bitset<std::numeric_limits<unsigned int>::digits> bs(itr);
        qDebug() << "itr:" << itr;
        for (int in = 0; in < inputs.size(); ++in) {
            float val = bs[in];
            dynamic_cast<Input *>(inputs[in])->setOn(not qFuzzyIsNull(val));
            float offset = (in_series.size() - in - 1 + out_series.size()) * 2 + gap + 0.5;
            in_series[in]->append(itr, static_cast<qreal>(offset + val));
            in_series[in]->append(itr + 1, static_cast<qreal>(offset + val));
        }
        qDebug() << "Updating the values of the circuit logic based on current input values.";
        sc->update();
        sc->updateAll();
        qDebug() << "Setting the computed output values to the waveform results.";
        int counter = 0;
        for (int out = 0; out < outputs.size(); ++out) {
            int inSz = outputs[out]->inputSize();
            for (int port = inSz - 1; port >= 0; --port) {
                float val = outputs[out]->input(port)->value() > 0;
                float offset = (out_series.size() - counter - 1) * 2 + 0.5;
                out_series[counter]->append(itr, static_cast<qreal>(offset + val));
                out_series[counter]->append(itr + 1, static_cast<qreal>(offset + val));
                /*        cout << counter << " " << out; */
                counter++;
            }
        }
    }
    qDebug() << "Inserting input series to the chart.";
    for (QLineSeries *in : qAsConst(in_series)) {
        m_chart.addSeries(in);
    }
    qDebug() << "Inserting output series to the chart.";
    for (QLineSeries *out : qAsConst(out_series)) {
        m_chart.addSeries(out);
    }
    qDebug() << "Setting graphic axes.";
    m_chart.createDefaultAxes();

    /*  chart.axisY( )->hide( ); */
    qDebug() << "Setting range and names to x, y axis.";
    const auto horizontal_axe = m_chart.axes(Qt::Horizontal);
    const auto vertical_axe = m_chart.axes(Qt::Vertical);

    auto *ax = dynamic_cast<QValueAxis *>(horizontal_axe.back());
    ax->setRange(0, num_iter);
    ax->setTickCount(num_iter + 1);
    ax->setLabelFormat(QString("%i"));
    auto *ay = dynamic_cast<QValueAxis *>(vertical_axe.back());
    /*  ay->setShadesBrush( QBrush( Qt::lightGray ) ); */

    qDebug() << "Setting graphics waveform color.";
    ay->setShadesColor(QColor(0, 0, 0, 8));
    ay->setShadesPen(QPen(QColor(0, 0, 0, 0)));
    ay->setShadesVisible(true);
    ay->setGridLineVisible(false);
    ay->setTickCount((in_series.size() + out_series.size() + gap / 2 + 1));
    ay->setRange(0, in_series.size() * 2 + out_series.size() * 2 + gap);
    ay->setGridLineColor(Qt::transparent);
    ay->setLabelsVisible(false);
    /*  ay->hide( ); */
    qDebug() << "Executing QDialog. Opens window to the user.";
    exec();
    qDebug() << "Restoring the old values to the inputs, prior to simulaton.";
    for (int in = 0; in < inputs.size(); ++in) {
        dynamic_cast<Input *>(inputs[in])->setOn(oldValues[in]);
    }
    qDebug() << "Resuming digital circuit main window after waveform simulation is finished.";
    scst.release();
}

void SimpleWaveform::on_radioButton_Position_clicked()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("waveform");
    m_sortingMode = SortingMode::POSITION;
    settings.setValue("sortingType", static_cast<int>(m_sortingMode));
    settings.endGroup();
    showWaveform();
}

void SimpleWaveform::on_radioButton_Increasing_clicked()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("waveform");
    m_sortingMode = SortingMode::INCREASING;
    settings.setValue("sortingType", static_cast<int>(m_sortingMode));
    settings.endGroup();
    showWaveform();
}

void SimpleWaveform::on_radioButton_Decreasing_clicked()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("waveform");
    m_sortingMode = SortingMode::DECREASING;
    settings.setValue("sortingType", static_cast<int>(m_sortingMode));
    settings.endGroup();
    showWaveform();
}

void SimpleWaveform::on_pushButton_Copy_clicked()
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
