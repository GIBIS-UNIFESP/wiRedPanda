// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/BeWavedDolphin.h"

#include <cmath>
#include <iostream>

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QPrinter>
#include <QSaveFile>
#include <QTextStream>

#include "App/BeWavedDolphin/Serializer.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/IO/Serialization.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Simulation/SimulationThrottleDisabler.h"
#include "App/UI/ClockDialog.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/LengthDialog.h"
#include "App/UI/MainWindow.h"
#include "App/UI/MessageDialog.h"

#ifdef USE_KDE_FRAMEWORKS
#include <KActionCollection>
#include <KStandardAction>
#include <KToolBar>
using namespace Qt::StringLiterals;
#endif

static constexpr int    kDefaultColumnWidth = 49;    ///<  Per-column pixel width at reset zoom.
static constexpr int    kMaxSimLength       = 2048;  ///<  Maximum allowed simulation length.
static constexpr int    kMaxScenePixels     = 4000;  ///<  Scene size limit before forced zoom reset.
static constexpr double kZoomStep           = 1.25;  ///<  Multiplicative factor per zoom step.

BewavedDolphin::BewavedDolphin(Scene *scene, const bool askConnection, MainWindow *parent)
    : BewavedDolphinBase(nullptr)
    , m_ui(std::make_unique<BewavedDolphinUi>())
    , m_mainWindow(parent)
    , m_externalScene(scene)
    // askConnection controls whether closing triggers a save-changes prompt and
    // whether saving offers to link the .dolphin file to the parent .panda file
    , m_askConnection(askConnection)
{
    m_ui->setupUi(this);
#ifdef USE_KDE_FRAMEWORKS
    setupKdeActions();
#endif
    m_ui->retranslateUi(this);

    // WA_DeleteOnClose ensures the window is freed when closed without the caller
    // needing to track its lifetime
    setAttribute(Qt::WA_DeleteOnClose);
    // Modal so the user cannot interact with the main circuit while the waveform is open
    setWindowModality(Qt::WindowModal);
    setWindowTitle(i18n("beWavedDolphin Simulator"));

    resize(800, 500);

    restoreGeometry(Settings::dolphinGeometry());

    // The delegate owns and renders the waveform pixmaps
    m_delegate = new SignalDelegate(this);
    m_signalTableView->setItemDelegate(m_delegate);

    // Embed the QTableView inside a QGraphicsScene so the whole waveform can be
    // uniformly scaled via the scene's transform without resizing individual rows/cols
    m_scene->addWidget(m_signalTableView);

    m_view.setScene(m_scene);
    // Scrollbars are suppressed because the scene is always resized to exactly fill the view
    m_view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // Redirect zoom events to BewavedDolphin so column widths are updated alongside the transform
    m_view.setRedirectZoom(true);
    m_ui->verticalLayout->addWidget(&m_view);

    m_ui->mainToolBar->setToolButtonStyle(Settings::labelsUnderIcons() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);

    connect(&m_view,                   &WaveformView::scaleIn,         this, &BewavedDolphin::on_actionZoomIn_triggered);
    connect(&m_view,                   &WaveformView::scaleOut,        this, &BewavedDolphin::on_actionZoomOut_triggered);
    connect(m_ui->actionAbout,         &QAction::triggered,            this, &BewavedDolphin::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,       &QAction::triggered,            this, &BewavedDolphin::on_actionAboutQt_triggered);
    connect(m_ui->actionClear,         &QAction::triggered,            this, &BewavedDolphin::on_actionClear_triggered);
    connect(m_ui->actionCombinational, &QAction::triggered,            this, &BewavedDolphin::on_actionCombinational_triggered);
    connect(m_ui->actionCopy,          &QAction::triggered,            this, &BewavedDolphin::on_actionCopy_triggered);
    connect(m_ui->actionCut,           &QAction::triggered,            this, &BewavedDolphin::on_actionCut_triggered);
    connect(m_ui->actionExit,          &QAction::triggered,            this, &BewavedDolphin::on_actionExit_triggered);
    connect(m_ui->actionExportToPdf,   &QAction::triggered,            this, &BewavedDolphin::on_actionExportToPdf_triggered);
    connect(m_ui->actionExportToPng,   &QAction::triggered,            this, &BewavedDolphin::on_actionExportToPng_triggered);
    connect(m_ui->actionFitScreen,     &QAction::triggered,            this, &BewavedDolphin::on_actionFitScreen_triggered);
    connect(m_ui->actionInvert,        &QAction::triggered,            this, &BewavedDolphin::on_actionInvert_triggered);
    connect(m_ui->actionLoad,          &QAction::triggered,            this, &BewavedDolphin::on_actionLoad_triggered);
    connect(m_ui->actionPaste,         &QAction::triggered,            this, &BewavedDolphin::on_actionPaste_triggered);
    connect(m_ui->actionResetZoom,     &QAction::triggered,            this, &BewavedDolphin::on_actionResetZoom_triggered);
    connect(m_ui->actionSave,          &QAction::triggered,            this, &BewavedDolphin::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,        &QAction::triggered,            this, &BewavedDolphin::on_actionSaveAs_triggered);
    connect(m_ui->actionSetClockWave,  &QAction::triggered,            this, &BewavedDolphin::on_actionSetClockWave_triggered);
    connect(m_ui->actionSetLength,     &QAction::triggered,            this, &BewavedDolphin::on_actionSetLength_triggered);
    connect(m_ui->actionSetTo0,        &QAction::triggered,            this, &BewavedDolphin::on_actionSetTo0_triggered);
    connect(m_ui->actionSetTo1,        &QAction::triggered,            this, &BewavedDolphin::on_actionSetTo1_triggered);
    connect(m_ui->actionShowNumbers,   &QAction::triggered,            this, &BewavedDolphin::on_actionShowNumbers_triggered);
    connect(m_ui->actionShowWaveforms, &QAction::triggered,            this, &BewavedDolphin::on_actionShowWaveforms_triggered);
    connect(m_ui->actionZoomIn,        &QAction::triggered,            this, &BewavedDolphin::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,       &QAction::triggered,            this, &BewavedDolphin::on_actionZoomOut_triggered);
    connect(m_ui->actionAutoCrop,      &QAction::triggered,            this, &BewavedDolphin::on_actionAutoCrop_triggered);
}

#ifdef USE_KDE_FRAMEWORKS
void BewavedDolphin::setupKdeActions()
{
    // Suppress "domain not set" warnings in tests — KAboutData sets this in main() for the
    // real app, but test binaries don't call main(), so set it explicitly here as a fallback.
    KLocalizedString::setApplicationDomain("wiredpanda");

    // KActionCollection::addAction() always overwrites the action's objectName with the
    // collection key. Set the legacy objectName AFTER addAction() to preserve findChild() compat.
    auto addAction = [this](const QString &kdeName, const QString &legacyName,
                             const QString &text, const QString &iconPath,
                             const QKeySequence &shortcut = {}) -> QAction * {
        auto *a = new QAction(this);
        a->setText(text);
        if (!iconPath.isEmpty()) a->setIcon(QIcon(iconPath));
        actionCollection()->addAction(kdeName, a);          // sets objectName = kdeName
        a->setObjectName(legacyName);                       // override AFTER to preserve findChild
        if (!shortcut.isEmpty()) actionCollection()->setDefaultShortcut(a, shortcut);
        return a;
    };

    auto stdAction = [this](QAction *a, const QString &legacyName, const QString &collectionName) {
        a->setParent(this);
        actionCollection()->addAction(collectionName, a);   // sets objectName = collectionName
        a->setObjectName(legacyName);                       // override AFTER
        return a;
    };

    // Create standard actions without callbacks — the constructor's connect() block
    // (which runs for both KDE and non-KDE paths) handles all signal-slot wiring.
    // Passing callbacks here would cause double-connections.
    using Obj = QObject *;
    using Slot = const char *;
    m_ui->actionLoad   = stdAction(KStandardAction::open(  static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionLoad"_s,   u"file_open"_s);
    m_ui->actionSave   = stdAction(KStandardAction::save(  static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionSave"_s,   u"file_save"_s);
    m_ui->actionSaveAs = stdAction(KStandardAction::saveAs(static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionSaveAs"_s, u"file_save_as"_s);
    m_ui->actionExit   = stdAction(KStandardAction::quit(  static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionExit"_s,   u"file_quit"_s);
    m_ui->actionCopy   = stdAction(KStandardAction::copy(  static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionCopy"_s,   u"edit_copy"_s);
    m_ui->actionCut    = stdAction(KStandardAction::cut(   static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionCut"_s,    u"edit_cut"_s);
    m_ui->actionPaste  = stdAction(KStandardAction::paste( static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionPaste"_s,  u"edit_paste"_s);
    m_ui->actionZoomIn = stdAction(KStandardAction::zoomIn(static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionZoomIn"_s, u"view_zoom_in"_s);
    m_ui->actionZoomOut= stdAction(KStandardAction::zoomOut(static_cast<Obj>(nullptr),static_cast<Slot>(nullptr), nullptr), u"actionZoomOut"_s,u"view_zoom_out"_s);

    m_ui->actionExportToPdf  = addAction(u"dolphin_export_pdf"_s,      u"actionExportToPdf"_s,   i18n("Export to PDF"),       u":/Interface/Dolphin/pdf.svg"_s,       QKeySequence(Qt::CTRL | Qt::Key_P));
    m_ui->actionExportToPng  = addAction(u"dolphin_export_png"_s,      u"actionExportToPng"_s,   i18n("Export to PNG"),       {},                                     QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    m_ui->actionClear        = addAction(u"dolphin_clear"_s,           u"actionClear"_s,         i18n("Clear"),               u":/Interface/Dolphin/reloadFile.svg"_s, QKeySequence(Qt::ALT | Qt::Key_X));
    m_ui->actionInvert       = addAction(u"dolphin_invert"_s,          u"actionInvert"_s,        i18n("Invert"),              {},                                     QKeySequence(Qt::Key_Space));
    m_ui->actionSetTo0       = addAction(u"dolphin_set0"_s,            u"actionSetTo0"_s,        i18n("Set to 0"),            {},                                     QKeySequence(Qt::Key_0));
    m_ui->actionSetTo1       = addAction(u"dolphin_set1"_s,            u"actionSetTo1"_s,        i18n("Set to 1"),            {},                                     QKeySequence(Qt::Key_1));
    m_ui->actionSetClockWave = addAction(u"dolphin_clock"_s,           u"actionSetClockWave"_s,  i18n("Set clock frequency"), {},                                     QKeySequence(Qt::ALT | Qt::Key_W));
    m_ui->actionSetLength    = addAction(u"dolphin_set_length"_s,      u"actionSetLength"_s,     i18n("Set Length"),          {},                                     QKeySequence(Qt::ALT | Qt::Key_L));
    // Merge/Split reserved for future multi-bit bus feature — always disabled
    m_ui->actionMerge = addAction(u"dolphin_merge"_s, u"actionMerge"_s, i18n("Merge"), {});
    m_ui->actionMerge->setEnabled(false);
    m_ui->actionSplit = addAction(u"dolphin_split"_s, u"actionSplit"_s, i18n("Split"), {});
    m_ui->actionSplit->setEnabled(false);
    m_ui->actionAutoCrop     = addAction(u"dolphin_auto_crop"_s,       u"actionAutoCrop"_s,      i18n("AutoCrop"),            u":/Interface/Dolphin/autoCrop.svg"_s,   QKeySequence(Qt::CTRL | Qt::Key_A));
    m_ui->actionCombinational= addAction(u"dolphin_combinational"_s,   u"actionCombinational"_s, i18n("Combinational"),       {},                                     QKeySequence(Qt::ALT | Qt::Key_C));
    m_ui->actionShowNumbers  = addAction(u"dolphin_show_numbers"_s,    u"actionShowNumbers"_s,   i18n("Show Numbers"),        {});
    m_ui->actionShowWaveforms= addAction(u"dolphin_show_waveforms"_s,  u"actionShowWaveforms"_s, i18n("Show Waveforms"),      {});
    m_ui->actionFitScreen    = addAction(u"dolphin_fit_screen"_s,      u"actionFitScreen"_s,     i18n("Fit to screen"),       u":/Interface/Dolphin/zoomRange.svg"_s,  QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    m_ui->actionResetZoom    = addAction(u"dolphin_reset_zoom"_s,      u"actionResetZoom"_s,     i18n("Reset Zoom"),          u":/Interface/Dolphin/zoomReset.svg"_s,  QKeySequence(Qt::CTRL | Qt::Key_Home));
    m_ui->actionAbout        = addAction(u"dolphin_about"_s,           u"actionAbout"_s,         i18n("About"),               u":/Interface/Dolphin/help.svg"_s);
    m_ui->actionAboutQt      = addAction(u"dolphin_about_qt"_s,        u"actionAboutQt"_s,       i18n("About Qt"),            {});

    // BewavedDolphin has no Ctrl+Alt+I or F1 conflict, but disable the auto
    // command bar / help menu for consistency with MainWindow.
    setCommandBarEnabled(false);
    setHelpMenuEnabled(false);

    setupGUI(KXmlGuiWindow::Default, u":/wiredpanda/bewaveddolphinui.rc"_s);

    // Grab toolbar and status bar
    m_ui->mainToolBar = qobject_cast<QToolBar *>(toolBar(u"mainToolBar"_s));
    m_ui->statusbar   = statusBar();

}
#endif // USE_KDE_FRAMEWORKS

BewavedDolphin::~BewavedDolphin()
{
    Settings::setDolphinGeometry(saveGeometry());
}

void BewavedDolphin::createWaveform(const QString &fileName)
{
    prepare(fileName);

    if (fileName.isEmpty()) {
        // No saved waveform — start with all-zero inputs and run once to populate outputs
        setWindowTitle(i18n("beWavedDolphin Simulator"));
        run();
    } else {
        // Resolve the file relative to the main window's working directory so that
        // relative paths stored inside .panda files resolve correctly
        QFileInfo fileInfo(m_mainWindow->currentDir(), QFileInfo(fileName).fileName());

        if (!fileInfo.exists()) {
            m_ui->statusbar->showMessage(i18n("File \"%1\" does not exist!", fileName), 4000);
            return;
        }

        load(fileInfo.absoluteFilePath());
    }

    qCDebug(zero) << "Resuming digital circuit main window after waveform simulation is finished.";
    // Reset edit flag — loading a file or a fresh run does not constitute a user edit
    m_edited = false;
}

void BewavedDolphin::createWaveform()
{
    prepare();
    loadFromTerminal();
}

void BewavedDolphin::loadFromTerminal()
{
    // Protocol: first line is "rows,cols"; subsequent lines contain comma-separated 0/1 values per row.
    // This allows driving the simulator from scripts without a GUI file dialog.
    QTextStream cin(stdin);
    QString str = cin.readLine();
    const auto wordList(str.split(','));

    if (wordList.size() < 2) {
        throw PANDACEPTION("");
    }

    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    // Clamp rows to the number of actual input ports to avoid out-of-bounds writes
    if (rows > m_inputPorts) {
        rows = m_inputPorts;
    }

    if ((cols < 2) || (cols > kMaxSimLength)) {
        throw PANDACEPTION("");
    }

    setLength(cols, false);

    for (int row = 0; row < rows; ++row) {
        str = cin.readLine();
        const auto wordList2(str.split(','));

        if (wordList2.size() < cols) {
            throw PANDACEPTION("");
        }

        for (int col = 0; col < cols; ++col) {
            const int value = wordList2.at(col).toInt();
            createElement(row, col, value, true, false);
        }
    }

    run();
}

void BewavedDolphin::prepare(const QString &fileName)
{
    qCDebug(zero) << "Updating window name with current: " << fileName;
    m_simulation = m_externalScene->simulation();

    qCDebug(zero) << "Loading elements. All elements initially in elements vector. Then, inputs and outputs are extracted from it.";
    loadElements();

    qCDebug(zero) << "Loading initial data into the table.";
    loadNewTable();
}

void BewavedDolphin::loadElements()
{
    m_inputs.clear();
    m_outputs.clear();
    m_inputPorts = 0;

    const auto elements = m_externalScene->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The circuit is empty. Add input and output elements to generate a waveform.");
    }

    for (auto *elm : elements) {
        if (!elm || (elm->type() != GraphicElement::Type)) {
            continue;
        }

        if (elm->elementGroup() == ElementGroup::Input) {
            m_inputs.append(qobject_cast<GraphicElementInput *>(elm));
            // Multi-bit inputs (e.g. rotary encoder) contribute multiple port rows
            m_inputPorts += elm->outputSize();
        }

        if (elm->elementGroup() == ElementGroup::Output) {
            m_outputs.append(elm);
        }
    }

    // Stable sort by label so the waveform table ordering is deterministic and
    // matches what the user expects from the label names they assigned
    std::stable_sort(m_inputs.begin(), m_inputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    std::stable_sort(m_outputs.begin(), m_outputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    if (m_inputs.isEmpty() && m_outputs.isEmpty()) {
        throw PANDACEPTION("The circuit has no input or output elements. Add at least one input (e.g. Switch) and one output (e.g. LED) to generate a waveform.");
    }

    if (m_inputs.isEmpty()) {
        throw PANDACEPTION("The circuit has no input elements. Add at least one input (e.g. Switch, Button, or Clock) to generate a waveform.");
    }

    if (m_outputs.isEmpty()) {
        throw PANDACEPTION("The circuit has no output elements. Add at least one output (e.g. LED or Display) to generate a waveform.");
    }
}

void BewavedDolphin::loadNewTable()
{
    qCDebug(zero) << "Getting initial value from inputs and writing them to oldvalues. Used to save current state of inputs and restore it after simulation. Not saving memory states though...";
    qCDebug(zero) << "Also getting the name of the inputs. If no label is given, the element type is used as a name.";
    QStringList inputLabels;
    QStringList outputLabels;
    // loadSignals also snapshots current input port values into m_oldInputValues so
    // they can be restored after the simulation sweep completes
    loadSignals(inputLabels, outputLabels);

    qCDebug(zero) << "Num iter = " << m_length;

    // Rows = total signals (inputs + outputs); columns = simulation length in time steps
    m_model = new SignalModel(static_cast<int>(inputLabels.size() + outputLabels.size()), m_length, this);
    m_signalTableView->setModel(m_model);

    // Input rows come first, then output rows — the split point is inputLabels.size()
    m_model->setVerticalHeaderLabels(inputLabels + outputLabels);

    m_signalTableView->setAlternatingRowColors(true);
    m_signalTableView->setShowGrid(false);

    // Fixed section sizes prevent the user from resizing columns, keeping waveforms aligned
    m_signalTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    m_signalTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);

    // Start at minimum width of 1px; zoom actions then set per-column widths to 49px
    m_signalTableView->horizontalHeader()->setDefaultSectionSize(1);

    qCDebug(zero) << "Inputs: " << inputLabels.size() << ", outputs: " << outputLabels.size();

    // Initialise all input cells to 0 and compute the first output sweep
    on_actionClear_triggered();

    connect(m_signalTableView,                   &QAbstractItemView::doubleClicked,      this, &BewavedDolphin::on_tableView_cellDoubleClicked);
    connect(m_signalTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BewavedDolphin::on_tableView_selectionChanged);
}

void BewavedDolphin::on_tableView_cellDoubleClicked()
{
    const auto indexes = m_signalTableView->selectionModel()->selectedIndexes();

    // Toggle each selected cell between 0 and 1, then re-simulate
    for (auto &index : indexes) {
        int value = m_model->index(index.row(), index.column(), QModelIndex()).data().toInt();
        value = (value + 1) % 2;
        createElement(index.row(), index.column(), value, true, false);
    }

    run();
}

void BewavedDolphin::on_tableView_selectionChanged()
{
    m_externalScene->clearSelection();

    const auto indexes = m_signalTableView->selectionModel()->selectedIndexes();

    // Highlight the corresponding input element in the circuit editor when the user
    // selects a waveform row, giving visual feedback about which signal they are editing.
    // Output rows (index >= m_inputs.size()) have no element to highlight.
    for (auto &index : indexes) {
        if (index.row() < m_inputs.size()) {
            m_inputs.at(index.row())->setSelected(true);
        }
    }

    m_externalScene->view()->update();
}

void BewavedDolphin::loadSignals(QStringList &inputLabels, QStringList &outputLabels)
{
    QVector<Status> oldValues(m_inputPorts);
    int oldIndex = 0;

    for (auto *input : std::as_const(m_inputs)) {
        QString label = input->label();

        // Fall back to the element type name when the user hasn't given it a label
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(input->elementType());
        }

        for (int port = 0; port < input->outputSize(); ++port) {
            // Multi-port inputs (e.g. bus inputs) get indexed labels like "A[0]", "A[1]"
            if (input->outputSize() > 1) {
                inputLabels.append(label + "[" + QString::number(port) + "]");
            } else {
                inputLabels.append(label);
            }

            // Snapshot the live port state before the simulation sweep overwrites it
            oldValues[oldIndex] = input->outputPort(port)->status();
            ++oldIndex;
        }
    }

    qCDebug(zero) << "Getting the name of the outputs. If no label is given, element type is used as a name.";

    for (auto *output : std::as_const(m_outputs)) {
        QString label = output->label();

        if (label.isEmpty()) {
            label = ElementFactory::translatedName(output->elementType());
        }

        // Outputs with multiple input ports (e.g. multi-bit displays) get indexed labels
        for (int port = 0; port < output->inputSize(); ++port) {
            if (output->inputSize() > 1) {
                outputLabels.append(label + "[" + QString::number(port) + "]");
            } else {
                outputLabels.append(label);
            }
        }
    }

    m_oldInputValues = oldValues;
}

void BewavedDolphin::run()
{
    // Block the live simulation timer while we drive the circuit manually column by column
    qCDebug(zero) << "Creating class to pause main window simulator while creating waveform.";
    SimulationBlocker simulationBlocker(m_simulation);
    // Disable the visual refresh throttle so phases 3–4 (port-status updates) run on every
    // update() call. Without this, output port statuses are stale for most columns and the
    // waveform shows incorrect values.
    SimulationThrottleDisabler throttleDisabler(m_simulation);

    // --- Step through each time column and compute circuit outputs ---
    for (int column = 0; column < m_model->columnCount(); ++column) {
        qCDebug(four) << "Itr: " << column << ", inputs: " << m_inputs.size();
        int row = 0;

        for (auto *input : std::as_const(m_inputs)) {
            const bool isRotary = qobject_cast<InputRotary *>(input);
            for (int port = 0; port < input->outputSize(); ++port) {
                const bool value = m_model->index(row++, column).data().toBool();

                // Rotary encoders only accept "setOn" for the high state; low is implicit
                if (isRotary && value) {
                    input->setOn(1, port);
                } else if (!isRotary) {
                    input->setOn(value, port);
                }
            }
        }

        qCDebug(four) << "Updating the values of the circuit logic based on current input values.";
        m_simulation->update();

        // Write computed output port states back into the model's output rows
        qCDebug(four) << "Setting the computed output values to the waveform results.";
        row = m_inputPorts;

        for (auto *output : std::as_const(m_outputs)) {
            for (int port = 0; port < output->inputSize(); ++port) {
                const int value = static_cast<int>(output->inputPort(port)->status());
                // isInput=false → green output pixmaps; changeNext=false → caller will refresh
                createElement(row, column, value, false, false);
                ++row;
            }
        }
    }

    // Restore the circuit to the state it was in before the sweep so the live simulation
    // resumes correctly when the SimulationBlocker is destroyed
    qCDebug(three) << "Setting inputs back to old values.";
    restoreInputs();
}

void BewavedDolphin::restoreInputs()
{
    qCDebug(zero) << "Restoring old values to inputs, prior to simulation.";

    // Uses m_oldInputValues captured at loadSignals() time. Note that index maps to
    // the element level, not the port level — multi-port inputs share the same saved value.
    for (int index = 0; index < m_inputs.size(); ++index) {
        auto *input = m_inputs.at(index);
        if (!input) {
            continue;
        }
        for (int port = 0; port < input->outputSize(); ++port) {
            const bool oldValue = static_cast<bool>(m_oldInputValues.at(index));

            if (input->outputSize() > 1) {
                input->setOn(oldValue, port);
            } else {
                input->setOn(oldValue);
            }
        }
    }
}

void BewavedDolphin::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    resizeScene();
}

void BewavedDolphin::resizeScene()
{
    const int newWidth  = m_ui->centralwidget->width();
    // Subtract 2px to avoid a persistent vertical scrollbar appearing at the boundary
    const int newHeight = m_ui->centralwidget->height() - 2;

    // kMaxScenePixels is a practical upper bound; beyond this Qt rendering becomes
    // unreliable and memory usage spikes. Reset zoom to recover a safe state.
    if (newWidth > kMaxScenePixels || newHeight > kMaxScenePixels) {
        on_actionResetZoom_triggered();
        throw PANDACEPTION("Waveform would be too big! Resetting zoom.");
    }

    setTableViewSize(newWidth, newHeight);
    m_scene->setSceneRect(m_scene->itemsBoundingRect());
}

void BewavedDolphin::setTableViewSize(const int width, const int height)
{
    // Divide by (m_scale * m_scaleFactor) so that when the scene applies its m_scale transform,
    // the resulting visible widget fills the central widget at ~80% of available space,
    // leaving room for the vertical header and a small margin
    m_signalTableView->resize(static_cast<int>(width  / (m_scale * m_scaleFactor)),
                              static_cast<int>(height / (m_scale * m_scaleFactor)));
}

void BewavedDolphin::on_actionExit_triggered()
{
    close();
}

void BewavedDolphin::closeEvent(QCloseEvent *event)
{
    (m_askConnection && checkSave()) ? event->accept() : event->ignore();
}

bool BewavedDolphin::checkSave()
{
    if (!m_edited) {
        return true;
    }

    auto reply =
            QMessageBox::question(
                this,
                i18n("wiRedPanda - beWavedDolphin"),
                i18n("Save simulation before closing?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (reply) {
    // After saving, check m_edited again: if the save itself failed (e.g. user dismissed
    // the Save As dialog), m_edited stays true and we should not allow the close
    case QMessageBox::Save:    on_actionSave_triggered(); return (!m_edited);
    case QMessageBox::Discard: return true;
    case QMessageBox::Cancel:  return false;
    default:                   return true;
    }
}

void BewavedDolphin::createElement(const int row, const int col, const int value, const bool isInput, const bool changeNext)
{
    setCellValue(row, col, value, isInput, changeNext);
}

void BewavedDolphin::setCellValue(const int row, const int col, const int value, const bool isInput, const bool changeNext)
{
    const auto index = m_model->index(row, col);
    const int currentValue = index.data().toInt();

    m_model->setData(index, value, Qt::DisplayRole);

    if (m_type == PlotType::Number) {
        m_model->setData(index, static_cast<uint>(Qt::AlignCenter), Qt::TextAlignmentRole);
    }

    if (m_type == PlotType::Line) {
        m_model->setData(index, static_cast<uint>(Qt::AlignLeft), Qt::TextAlignmentRole);

        const auto previousIndex = index.siblingAtColumn(col - 1);
        const bool hasPrev  = previousIndex.isValid();
        const int prevValue = hasPrev ? previousIndex.data().toInt() : 0;

        m_model->setData(index, m_delegate->pixmapFor(value, isInput, hasPrev, prevValue), Qt::DecorationRole);

        if (!changeNext) {
            return;
        }

        // If the value changed, the next cell's edge type (rising/falling) may have changed too
        const auto nextIndex = m_model->index(row, col + 1);
        if (nextIndex.isValid() && (currentValue != value)) {
            setCellValue(row, col + 1, nextIndex.data().toInt(), isInput, false);
        }
    }
}

void BewavedDolphin::show()
{
    QMainWindow::show();
    qCDebug(zero) << "Getting table dimensions.";
    resizeScene();
}

void BewavedDolphin::print()
{
    // Outputs in the same CSV format used by loadFromTerminal() / save(QSaveFile &),
    // allowing round-trip scripted use without a GUI
    std::cout << m_model->rowCount() << ",";
    std::cout << m_model->columnCount() << ",\n";

    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            std::cout << m_model->item(row, col)->text().toStdString() << ",";
        }

        std::cout << "\n";
    }
}

void BewavedDolphin::saveToTxt(QTextStream &stream)
{
    // Force combinational mode so the truth table covers all 2^n input combinations
    on_actionCombinational_triggered();

    // Expand the length to exactly the full truth table; capped at 2048 by setLength()
    const int truthTableSize = static_cast<int>(std::pow(2, m_inputPorts));
    setLength(truthTableSize, false);

    // Write input rows first, then output rows, each followed by its signal label
    for (int row = 0; row < m_inputs.size(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            stream << m_model->item(row, col)->text();
        }

        QString label = m_model->verticalHeaderItem(row)->text();
        stream << " : \"" << label << "\"\n";
    }

    stream << "\n";

    for (int row = static_cast<int>(m_inputs.size()); row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            stream << m_model->item(row, col)->text();
        }

        QString label = m_model->verticalHeaderItem(row)->text();
        stream << " : \"" << label << "\"\n";
    }
}

bool BewavedDolphin::exportToPng(const QString &filename)
{
    try {
        const int timeSteps   = m_model->columnCount();
        const int signalCount = m_model->rowCount();

        // Fixed cell size chosen to ensure waveform SVG segments are legible at 1:1 zoom
        const int cellWidth  = 50;   // pixels per time step
        const int cellHeight = 40;   // pixels per signal
        const int padding    = 100;  // extra space for labels and margins

        setTableViewSize((timeSteps * cellWidth) + padding, (signalCount * cellHeight) + padding);
        m_scene->setSceneRect(m_scene->itemsBoundingRect());

        return exportWaveformToPng(filename);
    } catch (...) {
        return false;
    }
}

void BewavedDolphin::applyToSelectedCells(const std::function<int(int)> &valueFn)
{
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    for (const auto &item : itemList) {
        setCellValue(item.row(), item.column(), valueFn(item.data().toInt()), true, true);
    }
    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionSetTo0_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Set cells to 0"));
    qCDebug(zero) << "Pressed 0.";
    applyToSelectedCells([](int) { return 0; });
}

void BewavedDolphin::on_actionSetTo1_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Set cells to 1"));
    qCDebug(zero) << "Pressed 1.";
    applyToSelectedCells([](int) { return 1; });
}

void BewavedDolphin::on_actionInvert_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Invert cells"));
    qCDebug(zero) << "Pressed Not.";
    applyToSelectedCells([](int v) { return (v + 1) % 2; });
}

int BewavedDolphin::sectionFirstColumn(const QItemSelection &ranges)
{
    int firstCol = m_model->columnCount() - 1;

    for (const auto &range : ranges) {
        if (range.left() < firstCol) {
            firstCol = range.left();
        }
    }

    return firstCol;
}

int BewavedDolphin::sectionFirstRow(const QItemSelection &ranges)
{
    int firstRow = m_model->rowCount() - 1;

    for (const auto &range : ranges) {
        if (range.top() < firstRow) {
            firstRow = range.top();
        }
    }

    return firstRow;
}

void BewavedDolphin::on_actionSetClockWave_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Set clock wave"));
    qCDebug(zero) << "Getting first column.";
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        throw PANDACEPTION("No cells selected.");
    }

    // Anchor the clock phase to the leftmost selected column so the waveform
    // starts at 0 regardless of where in the timeline the selection begins
    const int firstCol = sectionFirstColumn(ranges);

    qCDebug(zero) << "Setting the signal according to its column and clock period.";
    ClockDialog dialog(m_clockPeriod, this);
    const int clockPeriod = dialog.frequency();

    if (clockPeriod < 0) {
        return;
    }

    m_clockPeriod = clockPeriod;

    // First half of each period is LOW, second half is HIGH (50% duty cycle)
    const int halfClockPeriod = clockPeriod / 2;
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();

    for (const auto &item : itemList) {
        const int value = ((item.column() - firstCol) % clockPeriod < halfClockPeriod ? 0 : 1);
        setCellValue(item.row(), item.column(), value, true, true);
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionCombinational_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Combinational mode"));
    // Ensure the table is at least as long as the full truth table (2^n columns)
    const int truthTableSize = static_cast<int>((std::min)(static_cast<double>(kMaxSimLength), std::pow(2, m_inputPorts)));

    if (m_length < truthTableSize) {
        setLength(truthTableSize, false);
    }

    // Generate Gray-code-like input patterns: row 0 toggles every 1 column (period=2),
    // row 1 every 2 columns (period=4), etc. Together they enumerate all input combinations.
    qCDebug(zero) << "Setting the signal according to its columns and clock period.";
    int halfClockPeriod = 1;
    int clockPeriod     = 2;

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            setCellValue(row, col, (col % clockPeriod < halfClockPeriod ? 0 : 1), true, true);
        }

        // Double the period for each successive input bit; cap at max int-safe values
        halfClockPeriod = (std::min)(clockPeriod, 524288);
        clockPeriod     = (std::min)(2 * clockPeriod, 1048576);
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionSetLength_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Set length dialog"));
    qCDebug(zero) << "Setting the simulation length.";
    const int currentLength = m_length > 0 ? m_length : m_model->columnCount();
    LengthDialog dialog(currentLength, this);
    const int simLength = dialog.length();

    if (simLength < 0) {
        return;
    }

    setLength(simLength, true);
}

void BewavedDolphin::setLength(const int simLength, const bool runSimulation)
{
    if (m_length == simLength) {
        return;
    }

    m_length = simLength;

    if (simLength <= m_model->columnCount()) {
        // Shrinking: Qt's setColumnCount removes trailing columns automatically
        qCDebug(zero) << "Reducing or keeping the simulation length.";
        m_model->setColumnCount(simLength);
        resizeScene();
        m_edited = true;
        return;
    }

    // Growing: new input columns must be explicitly filled with zeros; output columns
    // are populated by run() and don't need pre-filling
    qCDebug(zero) << "Increasing the simulation length.";
    const int oldLength = m_model->columnCount();
    m_model->setColumnCount(simLength);

    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = oldLength; col < simLength; ++col) {
            // changeNext=false: avoid cascading into further new (still unset) cells
            setCellValue(row, col, 0, true, false);
        }
    }

    resizeScene();
    m_edited = true;
    qCDebug(zero) << "Running simulation.";

    if (runSimulation) {
        run();
    }
}

void BewavedDolphin::adjustColumnWidths(const std::function<int(int)> &widthFn)
{
    for (int col = 0; col < m_signalTableView->model()->columnCount(); ++col) {
        m_signalTableView->setColumnWidth(col, widthFn(m_signalTableView->columnWidth(col)));
    }
    resizeScene();
    zoomChanged();
}

void BewavedDolphin::on_actionZoomOut_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Zoom out"));
    m_view.zoomOut();
    // Shrink column widths to match the reduced scene scale so pixmaps still tile correctly
    adjustColumnWidths([this](int w) { return static_cast<int>(w / m_scale); });
}

void BewavedDolphin::on_actionZoomIn_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Zoom in"));
    m_view.zoomIn();
    // Grow column widths proportionally so waveform segments remain at their natural aspect ratio
    adjustColumnWidths([this](int w) { return static_cast<int>(w * m_scale); });
}

void BewavedDolphin::on_actionResetZoom_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Zoom reset"));
    m_view.resetZoom();
    m_scale = kZoomStep;
    adjustColumnWidths([](int) { return kDefaultColumnWidth; });
}

void BewavedDolphin::zoomChanged()
{
    m_ui->actionZoomIn->setEnabled(m_view.canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_view.canZoomOut());
}

void BewavedDolphin::on_actionFitScreen_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Fit screen"));
    // First undo the current scale transform so the measurements below are in logical pixels
    m_view.scale(1.0 / m_scale, 1.0 / m_scale);
    // Compute the scale needed to fit all columns and all rows within the current view
    const double wScale = static_cast<double>(m_view.width()) / (m_signalTableView->horizontalHeader()->length() + m_signalTableView->columnWidth(0));
    const double hScale = static_cast<double>(m_view.height()) / (m_signalTableView->verticalHeader()->length() + m_signalTableView->rowHeight(0) + 10);
    // Use the smaller scale so neither axis overflows the view
    m_scale = (std::min)(wScale, hScale);
    m_view.scale(1.0 * m_scale, 1.0 * m_scale);
    // FitScreen sets an arbitrary transform, so reset the discrete level to 0
    // so canZoomIn/Out limits are correct for subsequent zoom actions
    m_view.resetZoom();
    resizeScene();
    zoomChanged();
}

void BewavedDolphin::on_actionClear_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Clear input"));
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            setCellValue(row, col, 0, true, true);
        }
    }

    m_edited = true;
    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionAutoCrop_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Auto crop"));
    // Crop (or extend) the simulation to exactly the full truth table size for the
    // current number of input elements, then re-run to refresh output rows
    setLength(static_cast<int>(std::pow(2, m_inputs.length())), true);
}

void BewavedDolphin::on_actionCopy_triggered()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Waveform copy"));
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    // Serialise using a versioned header so paste can verify format compatibility
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writeDolphinHeader(stream);
    copy(ranges, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-bewaveddolphin-waveform", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void BewavedDolphin::copy(const QItemSelection &ranges, QDataStream &stream)
{
    qCDebug(zero) << "Serializing data into data stream.";
    const int firstRow = sectionFirstRow(ranges);
    const int firstCol = sectionFirstColumn(ranges);
    const auto itemList = m_signalTableView->selectionModel()->selectedIndexes();
    stream << itemList.size();

    for (const auto &item : itemList) {
        const int data_ = m_model->index(item.row(), item.column()).data().toInt();
        // Store offsets relative to the selection origin so paste can re-anchor
        // the data at any target cell regardless of absolute position
        stream << static_cast<qint64>(item.row() - firstRow);
        stream << static_cast<qint64>(item.column() - firstCol);
        stream << static_cast<qint64>(data_);
    }
}

void BewavedDolphin::on_actionCut_triggered()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Waveform cut"));
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writeDolphinHeader(stream);
    cut(ranges, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-bewaveddolphin-waveform", itemData);

    QApplication::clipboard()->setMimeData(mimeData);

    m_edited = true;
}

void BewavedDolphin::cut(const QItemSelection &ranges, QDataStream &stream)
{
    copy(ranges, stream);
    on_actionSetTo0_triggered();
}

void BewavedDolphin::on_actionPaste_triggered()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Waveform paste"));
    const auto ranges = m_signalTableView->selectionModel()->selection();

    if (ranges.isEmpty()) {
        return;
    }

    const auto *mimeData = QApplication::clipboard()->mimeData();
    QByteArray itemData;

    // Support both the legacy "bdolphin/copydata" MIME type and the current one,
    // so files or clipboard data from older versions of the app can still be pasted
    if (mimeData->hasFormat("bdolphin/copydata")) {
        itemData = mimeData->data("bdolphin/copydata");
    }

    if (mimeData->hasFormat("application/x-bewaveddolphin-waveform")) {
        itemData = mimeData->data("application/x-bewaveddolphin-waveform");
    }

    if (!itemData.isEmpty()) {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        Serialization::readDolphinHeader(stream);
        paste(ranges, stream);
        m_edited = true;
    }
}

void BewavedDolphin::paste(const QItemSelection &ranges, QDataStream &stream)
{
    const int firstCol = sectionFirstColumn(ranges);
    const int firstRow = sectionFirstRow(ranges);
    quint64 itemListSize; stream >> itemListSize;

    for (int i = 0; i < static_cast<int>(itemListSize); ++i) {
        quint64 row;   stream >> row;
        quint64 col;   stream >> col;
        quint64 data_; stream >> data_;
        // Re-anchor the stored relative offsets to the paste-target cell
        const int newRow = static_cast<int>(static_cast<quint64>(firstRow) + row);
        const int newCol = static_cast<int>(static_cast<quint64>(firstCol) + col);

        // Silently drop cells that land outside the input rows or past the simulation length;
        // output rows are never editable
        if ((newRow < m_inputPorts) && (newCol < m_model->columnCount())) {
            setCellValue(newRow, newCol, static_cast<int>(data_), true, true);
        }
    }

    run();
}

void BewavedDolphin::on_actionSave_triggered()
{
    sentryBreadcrumb("file", QStringLiteral("Waveform save"));
    if (m_currentFile.fileName().isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }

    save(m_currentFile.absoluteFilePath());
    m_ui->statusbar->showMessage(i18n("Saved file successfully."), 4000);
    m_edited = false;
}

void BewavedDolphin::on_actionSaveAs_triggered()
{
    sentryBreadcrumb("file", QStringLiteral("Waveform save as"));
    const QString path = m_mainWindow->currentFile().absolutePath();

    // List the format that matches the current file first so it is the default selection
    const QString fileFilter = m_currentFile.fileName().endsWith(".csv") ?
                i18n("CSV files (*.csv);;Dolphin files (*.dolphin);;All supported files (*.dolphin *.csv)")
              : i18n("Dolphin files (*.dolphin);;CSV files (*.csv);;All supported files (*.dolphin *.csv)");

    const auto result = FileDialogs::provider()->getSaveFileName(this, i18n("Save File as..."), path, fileFilter);
    QString fileName = result.fileName;

    if (fileName.isEmpty()) {
        return;
    }

    // Append the correct extension when the user types a bare name without one,
    // inferring the format from whichever filter was active in the dialog
    if (!fileName.endsWith(".dolphin") && !fileName.endsWith(".csv")) {
        if (result.selectedFilter.contains("dolphin")) {
            fileName.append(".dolphin");
        } else {
            fileName.append(".csv");
        }
    }

    save(fileName);
    m_currentFile = QFileInfo(fileName);
    associateToWiRedPanda(fileName);
    setWindowTitle(i18n("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
    m_ui->statusbar->showMessage(i18n("Saved file successfully."), 4000);
    m_edited = false;
}

void BewavedDolphin::save(const QString &fileName)
{
    // QSaveFile writes to a temp file and atomically renames on commit,
    // preventing data loss if the process is interrupted during a write
    QSaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Error opening file: %1", file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << "Saving dolphin file.";
        QDataStream stream(&file);
        Serialization::writeDolphinHeader(stream);
        save(stream);
    } else {
        qCDebug(zero) << "Saving CSV file.";
        save(file);
    }

    if (!file.commit()) {
        throw PANDACEPTION("Error saving file: %1", file.errorString());
    }
}

void BewavedDolphin::save(QDataStream &stream)
{
    qCDebug(zero) << "Serializing data into data stream.";
    DolphinSerializer::saveBinary(stream, m_model, m_inputPorts);
}

void BewavedDolphin::save(QSaveFile &file)
{
    DolphinSerializer::saveCSV(file, m_model);
}

void BewavedDolphin::associateToWiRedPanda(const QString &fileName)
{
    // Only prompt when the file is new (not already linked) and we are in interactive mode;
    // non-interactive (command-line / test) sessions skip the dialog entirely
    if ((m_mainWindow->dolphinFileName() != fileName) && Application::interactiveMode) {
        if (MessageDialog::questionYesNo(this,
                i18n("Do you want to link this beWavedDolphin file to your current wiRedPanda file and save it?"),
                i18n("wiRedPanda - beWavedDolphin"))) {
            m_mainWindow->setDolphinFileName(fileName);
            m_mainWindow->save();
        }
    }
}

void BewavedDolphin::on_actionLoad_triggered()
{
    sentryBreadcrumb("file", QStringLiteral("Waveform load dialog"));
    QDir defaultDirectory;

    // Prefer the last-used dolphin file's directory; fall back to the main window's
    // working directory, and finally to the home directory
    if (m_currentFile.exists()) {
        defaultDirectory.setPath(m_currentFile.absolutePath());
    } else {
        if (m_mainWindow->currentFile().exists()) {
            m_mainWindow->currentFile().dir();
        } else {
            defaultDirectory.setPath(QDir::homePath());
        }
    }

    const QString homeDir(m_mainWindow->currentDir().absolutePath());

    const QString fileName = FileDialogs::provider()->getOpenFileName(
        this, i18n("Open File"), homeDir,
        i18n("All supported files (*.dolphin *.csv);;Dolphin files (*.dolphin);;CSV files (*.csv)"));

    if (fileName.isEmpty()) {
        return;
    }

    load(fileName);
    m_edited = false;
    m_ui->statusbar->showMessage(i18n("File loaded successfully."), 4000);
}

void BewavedDolphin::load(const QString &fileName)
{
    QFile file(fileName);

    if (!file.exists()) {
        throw PANDACEPTION("File \"%1\" does not exist!", fileName);
    }

    qCDebug(zero) << "File exists.";

    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << "Could not open file in ReadOnly mode: " << file.errorString();
        throw PANDACEPTION("Could not open file for reading: %1", file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << "Dolphin file opened.";
        QDataStream stream(&file);
        Serialization::readDolphinHeader(stream);
        qCDebug(zero) << "Loading in editor.";
        load(stream);
        qCDebug(zero) << "Current file set.";
        m_currentFile = QFileInfo(fileName);
    } else if (fileName.endsWith(".csv")) {
        qCDebug(zero) << "CSV file opened.";
        qCDebug(zero) << "Loading in editor.";
        load(file);
        qCDebug(zero) << "Current file set.";
        m_currentFile = QFileInfo(fileName);
    } else {
        qCDebug(zero) << "Format not supported. Could not open file: " << fileName;
        throw PANDACEPTION("Format not supported. Could not open file: %1", fileName);
    }

    qCDebug(zero) << "Closing file.";
    file.close();
    associateToWiRedPanda(fileName);
    setWindowTitle(i18n("beWavedDolphin Simulator") + " [" + m_currentFile.fileName() + "]");
}

void BewavedDolphin::load(QDataStream &stream)
{
    const auto loadedData = DolphinSerializer::loadBinary(stream, m_inputPorts);
    setLength(loadedData.columns, false);
    qCDebug(zero) << "Update table.";

    for (int row = 0; row < loadedData.inputPorts; ++row) {
        for (int col = 0; col < loadedData.columns; ++col) {
            setCellValue(row, col, loadedData.values[row * loadedData.columns + col], true, true);
        }
    }

    run();
}

void BewavedDolphin::load(QFile &file)
{
    const auto loadedData = DolphinSerializer::loadCSV(file, m_inputPorts);
    setLength(loadedData.columns, false);
    qCDebug(zero) << "Update table.";

    for (int row = 0; row < loadedData.inputPorts; ++row) {
        for (int col = 0; col < loadedData.columns; ++col) {
            setCellValue(row, col, loadedData.values[row * loadedData.columns + col], true, true);
        }
    }

    run();
}

void BewavedDolphin::on_actionShowNumbers_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Show numbers"));
    m_type = PlotType::Number;

    // Clear the DecorationRole pixmaps that the Line mode sets; if left, the delegate
    // would paint the waveform image on top of the numeric text
    for (int row = 0; row < m_model->rowCount(); ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            m_model->setData(m_model->index(row, col), QVariant(), Qt::DecorationRole);
        }
    }

    // Re-apply input cells through setCellValue so alignment roles are set for Number mode
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            setCellValue(row, col, m_model->index(row, col).data().toInt(), true, true);
        }
    }

    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionShowWaveforms_triggered()
{
    sentryBreadcrumb("waveform", QStringLiteral("Show waveforms"));
    m_type = PlotType::Line;

    // Re-create all input cells so they receive the correct rising/falling pixmaps
    // for the current value sequence; output cells are refreshed by run() below
    for (int row = 0; row < m_inputPorts; ++row) {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            setCellValue(row, col, m_model->index(row, col).data().toInt(), true, true);
        }
    }

    qCDebug(zero) << "Running simulation.";
    run();
}

void BewavedDolphin::on_actionExportToPng_triggered()
{
    sentryBreadcrumb("export", QStringLiteral("Waveform export PNG"));
    QString pngFile = FileDialogs::provider()->getSaveFileName(this, i18n("Export to Image"), m_currentFile.absolutePath(), i18n("PNG files (*.png)")).fileName;

    if (pngFile.isEmpty()) {
        return;
    }

    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }

    exportWaveformToPng(pngFile);
}

bool BewavedDolphin::exportWaveformToPng(const QString &filename)
{
    const QRectF sceneRect = m_scene->sceneRect();
    // Create a pixmap exactly the size of the scene so there is no blank padding
    QPixmap pixmap(sceneRect.size().toSize());

    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    // Render the full scene into the pixmap without any additional clipping
    m_scene->render(&painter, QRectF(), sceneRect);
    painter.end();

    return pixmap.toImage().save(filename);
}

void BewavedDolphin::on_actionExportToPdf_triggered()
{
    sentryBreadcrumb("export", QStringLiteral("Waveform export PDF"));
    QString pdfFile = FileDialogs::provider()->getSaveFileName(this, i18n("Export to PDF"), m_currentFile.absolutePath(), i18n("PDF files (*.pdf)")).fileName;

    if (pdfFile.isEmpty()) {
        return;
    }

    if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
        pdfFile.append(".pdf");
    }

    // Landscape A4 fits a reasonably long waveform without excessive scaling
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFile);

    QPainter painter;

    if (!painter.begin(&printer)) {
        throw PANDACEPTION("Could not print this circuit to PDF.");
    }

    // Expand the source rect by 64px on each side to add a small margin around the waveform
    m_scene->render(&painter, QRectF(), m_scene->sceneRect().adjusted(-64, -64, 64, 64));
    painter.end();
}

void BewavedDolphin::on_actionAbout_triggered()
{
    QMessageBox::about(this,
        "beWavedDolphin",
        i18n("<p>beWavedDolphin is a waveform simulator for wiRedPanda, developed by the Federal University of São Paulo"
           " to help students learn about logic circuits.</p>"
           "<p>Software version: %1</p>"
           "<p><strong>Creators:</strong></p>"
           "<ul>"
           "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
           "</ul>"
           "<p> beWavedDolphin is currently maintained by Prof. Fábio Cappabianco, Ph.D. and his students.</p>"
           "<p> Please file a report at our GitHub page if you find a bug or want to request a new feature.</p>"
           "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
            .arg(QApplication::applicationVersion()));
}

void BewavedDolphin::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

