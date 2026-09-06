// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/CircuitRecorderDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "App/Core/Settings.h"
#include "App/UI/FileDialogProvider.h"

CircuitRecorderDialog::CircuitRecorderDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("viVIDFrog"));
    setWindowIcon(QIcon(QStringLiteral(":/Interface/Toolbar/vividfrog_icon.svg")));
    resize(480, 260);
    setupUI();
}

void CircuitRecorderDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    // Simulation Mode
    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem(tr("Live Simulation (Panda)"), static_cast<int>(CircuitRecorder::SimulationMode::Live));
    m_modeCombo->addItem(tr("Dolphin Waveform (.dolphin)"), static_cast<int>(CircuitRecorder::SimulationMode::DolphinWaveform));
    formLayout->addRow(tr("Simulation Mode:"), m_modeCombo);

    // Dolphin Container
    m_dolphinContainer = new QWidget(this);
    auto *dolphinLayout = new QFormLayout(m_dolphinContainer);
    dolphinLayout->setContentsMargins(0, 0, 0, 0);

    auto *dolphinFileLayout = new QHBoxLayout();
    m_dolphinPathEdit = new QLineEdit(m_dolphinContainer);
    m_dolphinPathEdit->setPlaceholderText(tr("Select .dolphin or .csv waveform file..."));
    m_dolphinBrowseButton = new QPushButton(tr("Browse..."), m_dolphinContainer);
    dolphinFileLayout->addWidget(m_dolphinPathEdit);
    dolphinFileLayout->addWidget(m_dolphinBrowseButton);
    dolphinLayout->addRow(tr("Waveform File:"), dolphinFileLayout);

    m_stepDurationSpinBox = new QDoubleSpinBox(m_dolphinContainer);
    m_stepDurationSpinBox->setRange(0.1, 10.0);
    m_stepDurationSpinBox->setValue(1.0);
    m_stepDurationSpinBox->setSingleStep(0.25);
    m_stepDurationSpinBox->setSuffix(tr(" s"));
    dolphinLayout->addRow(tr("Step Hold Duration:"), m_stepDurationSpinBox);

    m_autoTrimCheckBox = new QCheckBox(tr("Auto-trim trailing unchanged steps"), m_dolphinContainer);
    m_autoTrimCheckBox->setChecked(true);
    dolphinLayout->addRow(m_autoTrimCheckBox);

    formLayout->addRow(m_dolphinContainer);
    m_dolphinContainer->setVisible(false);

    // File path
    auto *fileLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setPlaceholderText(tr("Select output file path..."));
    m_browseButton = new QPushButton(tr("Browse..."), this);
    fileLayout->addWidget(m_filePathEdit);
    fileLayout->addWidget(m_browseButton);
    formLayout->addRow(tr("Output File:"), fileLayout);

    // Format
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItem(tr("Animated GIF (*.gif)"), static_cast<int>(CircuitRecorder::Format::GIF));

    bool ffmpegAvailable = CircuitRecorder::isFFmpegAvailable();
    int mp4Index = m_formatCombo->count();
    m_formatCombo->addItem(tr("MP4 Video (*.mp4)"), static_cast<int>(CircuitRecorder::Format::MP4));
    if (!ffmpegAvailable) {
        m_formatCombo->setItemData(mp4Index, tr("Requires FFmpeg on system PATH"), Qt::ToolTipRole);
        m_formatCombo->setItemData(mp4Index, 0, Qt::UserRole - 1);
    }

    int webmIndex = m_formatCombo->count();
    m_formatCombo->addItem(tr("WebM Video (*.webm)"), static_cast<int>(CircuitRecorder::Format::WebM));
    if (!ffmpegAvailable) {
        m_formatCombo->setItemData(webmIndex, tr("Requires FFmpeg on system PATH"), Qt::ToolTipRole);
        m_formatCombo->setItemData(webmIndex, 0, Qt::UserRole - 1);
    }

    formLayout->addRow(tr("Format:"), m_formatCombo);

    // Region
    m_regionCombo = new QComboBox(this);
    m_regionCombo->addItem(tr("Full Circuit"), static_cast<int>(CircuitRecorder::Region::FullCircuit));
    m_regionCombo->addItem(tr("Visible Viewport"), static_cast<int>(CircuitRecorder::Region::VisibleViewport));
    m_regionCombo->addItem(tr("Selected Region"), static_cast<int>(CircuitRecorder::Region::Selection));
    formLayout->addRow(tr("Capture Region:"), m_regionCombo);

    // FPS
    m_fpsSpinBox = new QSpinBox(this);
    m_fpsSpinBox->setRange(10, 30);
    m_fpsSpinBox->setValue(15);
    m_fpsSpinBox->setSuffix(tr(" FPS"));
    formLayout->addRow(tr("Frame Rate:"), m_fpsSpinBox);

    // Scale
    m_scaleCombo = new QComboBox(this);
    m_scaleCombo->addItem(tr("100% (Original)"), 1.0);
    m_scaleCombo->addItem(tr("75% Scale"), 0.75);
    m_scaleCombo->addItem(tr("50% Scale"), 0.5);
    formLayout->addRow(tr("Resolution Scale:"), m_scaleCombo);

    mainLayout->addLayout(formLayout);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_recordButton = new QPushButton(tr("Start Recording"), this);
    m_recordButton->setDefault(true);

    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_recordButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CircuitRecorderDialog::onModeChanged);
    connect(m_dolphinBrowseButton, &QPushButton::clicked, this, &CircuitRecorderDialog::browseDolphinFile);
    connect(m_browseButton, &QPushButton::clicked, this, &CircuitRecorderDialog::browseOutputFile);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CircuitRecorderDialog::updateFormatExtension);
    connect(m_recordButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Default output path
    QString initialPath = Settings::lastRecordingPath();
    if (initialPath.isEmpty()) {
        initialPath = QDir::homePath() + QStringLiteral("/circuit_recording.gif");
    }
    m_filePathEdit->setText(initialPath);
}

void CircuitRecorderDialog::accept()
{
    if (m_filePathEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Missing Output File"),
                             tr("Please specify a valid output file path before starting the recording."));
        m_filePathEdit->setFocus();
        return;
    }

    auto mode = static_cast<CircuitRecorder::SimulationMode>(m_modeCombo->currentData().toInt());
    if (mode == CircuitRecorder::SimulationMode::DolphinWaveform) {
        QString dolphinPath = m_dolphinPathEdit->text().trimmed();
        if (dolphinPath.isEmpty() || !QFile::exists(dolphinPath)) {
            QMessageBox::warning(this, tr("Missing Waveform File"),
                                 tr("Please select an existing .dolphin or .csv waveform file before starting the recording."));
            m_dolphinPathEdit->setFocus();
            return;
        }
    }

    QDialog::accept();
}

void CircuitRecorderDialog::onModeChanged(int index)
{
    (void)index;
    auto mode = static_cast<CircuitRecorder::SimulationMode>(m_modeCombo->currentData().toInt());
    m_dolphinContainer->setVisible(mode == CircuitRecorder::SimulationMode::DolphinWaveform);
    adjustSize();
}

void CircuitRecorderDialog::browseDolphinFile()
{
    QString selectedFile = FileDialogs::provider()->getOpenFileName(
        this, tr("Select Dolphin Waveform File"), m_dolphinPathEdit->text(),
        tr("Dolphin Waveforms (*.dolphin *.csv);;All Files (*)"));
    if (!selectedFile.isEmpty()) {
        m_dolphinPathEdit->setText(selectedFile);
    }
}

void CircuitRecorderDialog::browseOutputFile()
{
    QString filter;
    auto format = static_cast<CircuitRecorder::Format>(m_formatCombo->currentData().toInt());
    switch (format) {
    case CircuitRecorder::Format::MP4:
        filter = tr("MP4 Video (*.mp4)");
        break;
    case CircuitRecorder::Format::WebM:
        filter = tr("WebM Video (*.webm)");
        break;
    case CircuitRecorder::Format::GIF:
    default:
        filter = tr("Animated GIF (*.gif)");
        break;
    }

    QString selectedFile = FileDialogs::provider()->getSaveFileName(
        this, tr("Save Recording As"), m_filePathEdit->text(), filter).fileName;
    if (!selectedFile.isEmpty()) {
        m_filePathEdit->setText(selectedFile);
    }
}

void CircuitRecorderDialog::updateFormatExtension()
{
    QString current = m_filePathEdit->text();
    if (current.isEmpty()) {
        return;
    }

    qsizetype lastDot = current.lastIndexOf(QLatin1Char('.'));
    if (lastDot != -1) {
        current = current.left(lastDot);
    }

    auto format = static_cast<CircuitRecorder::Format>(m_formatCombo->currentData().toInt());
    switch (format) {
    case CircuitRecorder::Format::MP4:
        current += QStringLiteral(".mp4");
        break;
    case CircuitRecorder::Format::WebM:
        current += QStringLiteral(".webm");
        break;
    case CircuitRecorder::Format::GIF:
    default:
        current += QStringLiteral(".gif");
        break;
    }
    m_filePathEdit->setText(current);
}

CircuitRecorder::Configuration CircuitRecorderDialog::configuration() const
{
    CircuitRecorder::Configuration config;
    config.filePath = m_filePathEdit->text();
    config.format = static_cast<CircuitRecorder::Format>(m_formatCombo->currentData().toInt());
    config.region = static_cast<CircuitRecorder::Region>(m_regionCombo->currentData().toInt());
    config.fps = m_fpsSpinBox->value();
    config.resolutionScale = m_scaleCombo->currentData().toDouble();
    config.simulationMode = static_cast<CircuitRecorder::SimulationMode>(m_modeCombo->currentData().toInt());
    config.dolphinFilePath = m_dolphinPathEdit->text();
    config.stepDurationSeconds = m_stepDurationSpinBox->value();
    config.autoTrimTrailingSteps = m_autoTrimCheckBox->isChecked();
    return config;
}

void CircuitRecorderDialog::setConfiguration(const CircuitRecorder::Configuration &config)
{
    m_filePathEdit->setText(config.filePath);

    int modeIdx = m_modeCombo->findData(static_cast<int>(config.simulationMode));
    if (modeIdx != -1) {
        m_modeCombo->setCurrentIndex(modeIdx);
    }

    if (!config.dolphinFilePath.isEmpty()) {
        m_dolphinPathEdit->setText(config.dolphinFilePath);
        int dolphinModeIdx = m_modeCombo->findData(static_cast<int>(CircuitRecorder::SimulationMode::DolphinWaveform));
        if (dolphinModeIdx != -1) {
            m_modeCombo->setCurrentIndex(dolphinModeIdx);
        }
    }

    m_stepDurationSpinBox->setValue(config.stepDurationSeconds);
    m_autoTrimCheckBox->setChecked(config.autoTrimTrailingSteps);

    int formatIdx = m_formatCombo->findData(static_cast<int>(config.format));
    if (formatIdx != -1) {
        m_formatCombo->setCurrentIndex(formatIdx);
    }

    int regionIdx = m_regionCombo->findData(static_cast<int>(config.region));
    if (regionIdx != -1) {
        m_regionCombo->setCurrentIndex(regionIdx);
    }

    m_fpsSpinBox->setValue(config.fps);

    int scaleIdx = m_scaleCombo->findData(config.resolutionScale);
    if (scaleIdx != -1) {
        m_scaleCombo->setCurrentIndex(scaleIdx);
    }
}
