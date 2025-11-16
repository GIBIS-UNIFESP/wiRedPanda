// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSlider>

/*!
 * @class LabeledSlider
 * @brief A custom QSlider that displays numeric labels under tick marks
 *
 * This widget extends QSlider to show labels under each tick mark,
 * making it easier to understand the slider values at a glance.
 */
class LabeledSlider : public QSlider
{
    Q_OBJECT

public:
    explicit LabeledSlider(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};
