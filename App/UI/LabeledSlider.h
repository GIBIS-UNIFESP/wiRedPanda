// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief LabeledSlider: QSlider subclass that draws numeric labels beneath each tick mark.
 */

#pragma once

#include <QSlider>

/**
 * \class LabeledSlider
 * \brief QSlider that draws numeric value labels beneath each tick mark.
 *
 * \details Used in the ElementEditor delay slider so users can see exact values
 * without relying on tooltips.
 */
class LabeledSlider : public QSlider
{
    Q_OBJECT

public:
    /// Constructs the labeled slider with horizontal orientation.
    explicit LabeledSlider(QWidget *parent = nullptr);

protected:
    // --- Painting ---

    /// \reimp Draws the slider track and tick labels.
    void paintEvent(QPaintEvent *event) override;
};

