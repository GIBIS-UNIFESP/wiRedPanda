// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the 7-segment LED display.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class Display7
 * \brief 7-segment LED display output element (7 segments + decimal point = 8 inputs).
 *
 * \details Each input port controls one segment (a–g + dp).  Supports red, green,
 * and blue color variants.
 */
class Display7 : public Element
{
    Q_OBJECT

public:
    /// Default segment color name. Single source of truth: used by the colour's
    /// spec and serialization's default-elision check.
    static constexpr const char *kDefaultColor = "Red";

    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Display7(QGraphicsItem *parent = nullptr);

    /// One segment input of a display: where it sits, its letter and its description, marked with QT_TR_NOOP() in the
    /// display's own code.
    struct SegmentPort {
        qreal x;
        qreal y;
        const char *letter;
        const char *description;
    };

    /**
     * \brief Returns the attributes of the segment input \a segment, named by \a described.
     * \details \a described is the display's describedPort() of the segment's letter and description. Segments are
     * individually driven: the input is optional, and leaving it unconnected turns the segment off.
     */
    static PortAttributes segmentInput(PortAttributes described, const SegmentPort &segment);

    // --- Visual ---

    /// Refreshes the displayed segments based on current input states.
    void refresh() override;

    /// The 8 segment inputs, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;
};
