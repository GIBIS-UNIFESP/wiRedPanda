// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"

/**
 * \brief CPU Building Blocks Test Utilities
 *
 * Common utility functions and constants for testing CPU building block ICs.
 * Reduces code duplication for reading output values and magic numbers.
 */
namespace CPUTestUtils {

// ============================================================================
// Test Pattern Constants - 8-bit values used for testing
// ============================================================================

/// All zeros pattern: 00000000
constexpr int PATTERN_ALL_ZEROS = 0x00;

/// All ones pattern: 11111111
constexpr int PATTERN_ALL_ONES = 0xFF;

/// Alternating bits pattern 1: 10101010
constexpr int PATTERN_ALTERNATING_1 = 0xAA;

/// Alternating bits pattern 2: 01010101
constexpr int PATTERN_ALTERNATING_2 = 0x55;

/// Lower nibble pattern: 00001111
constexpr int PATTERN_LOWER_NIBBLE = 0x0F;

/// Upper nibble pattern: 11110000
constexpr int PATTERN_UPPER_NIBBLE = 0xF0;

/// Pattern with bits 0,2,4,6 set: 01010101
constexpr int PATTERN_EVEN_BITS = 0x55;

/// Pattern with bits 1,3,5,7 set: 10101010
constexpr int PATTERN_ODD_BITS = 0xAA;

/**
 * \brief Get 8-bit output value from C-style array of LEDs
 *
 * Convenience overload for C-style arrays (e.g., Led* q[8]).
 * Converts the array to a QVector and reads the output.
 *
 * \param leds C-style array of 8 LED pointers
 * \return 8-bit value (0-255) from the LED array
 */
template<size_t N>
inline int get8BitValue(Led* (&leds)[N]) {
    QVector<GraphicElement *> elements;
    for (size_t i = 0; i < N; ++i) {
        elements.append(leds[i]);
    }
    return TestUtils::readMultiBitOutput(elements, 0);
}

/**
 * \brief Load a CPU Building Block IC from test component directory
 *
 * Loads IC from Tests/Integration/IC/Components/ and handles working
 * directory changes for nested IC resolution.
 *
 * \param filename IC filename (e.g., "cpu_level3_alu.panda")
 * \return Pointer to loaded IC (caller owns memory)
 * @throws std::runtime_error if file not found or load fails
 */
inline IC* loadBuildingBlockIC(const QString &filename) {
    auto ic = std::make_unique<IC>();
    QString absoluteIcPath = TestUtils::cpuComponentsDir() + filename;
    QString icDirectory = QFileInfo(absoluteIcPath).absolutePath();

    // Set working directory to the IC's own directory for nested IC resolution
    GlobalProperties::currentDir = icDirectory;
    try {
        ic->loadFile(absoluteIcPath);
    } catch (const Pandaception &e) {
        throw std::runtime_error(
            QString("Failed to load IC %1: %2").arg(filename, e.what()).toStdString()
        );
    }

    return ic.release();
}
} // namespace CPUTestUtils
