// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CircuitRecorderDialog: Configures output options for circuit simulation recording.
 */

#pragma once

#include <QDialog>

#include "App/UI/CircuitRecorder.h"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QPushButton;

/**
 * \class CircuitRecorderDialog
 * \brief Dialog allowing users to select file path, format, frame rate, region, and scale.
 */
class CircuitRecorderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CircuitRecorderDialog(QWidget *parent = nullptr);
    ~CircuitRecorderDialog() override = default;

    /// Returns the configured recording options.
    CircuitRecorder::Configuration configuration() const;

    /// Preset initial configuration.
    void setConfiguration(const CircuitRecorder::Configuration &config);

    void accept() override;

private slots:
    void browseOutputFile();
    void browseDolphinFile();
    void updateFormatExtension();
    void onModeChanged(int index);

private:
    void setupUI();

    QComboBox *m_modeCombo = nullptr;
    QWidget *m_dolphinContainer = nullptr;
    QLineEdit *m_dolphinPathEdit = nullptr;
    QPushButton *m_dolphinBrowseButton = nullptr;
    QDoubleSpinBox *m_stepDurationSpinBox = nullptr;
    QCheckBox *m_autoTrimCheckBox = nullptr;

    QLineEdit *m_filePathEdit = nullptr;
    QPushButton *m_browseButton = nullptr;
    QComboBox *m_formatCombo = nullptr;
    QComboBox *m_regionCombo = nullptr;
    QSpinBox *m_fpsSpinBox = nullptr;
    QComboBox *m_scaleCombo = nullptr;
    QPushButton *m_recordButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
};
