// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestMultiplexing.h"

#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/Mux.h"
#include "Tests/Common/TestUtils.h"

void TestMultiplexing::testMux()
{
    Mux elm;
    QCOMPARE(elm.inputSize(), 3);      // Starts at minimum (1 data + 2 select for 2:1)
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 3);
    QCOMPARE(elm.maxInputSize(), 11);  // Now supports up to 8:1 (8 data + 3 select)
    QCOMPARE(elm.elementType(), ElementType::Mux);
}

void TestMultiplexing::testDemux()
{
    Demux elm;
    QCOMPARE(elm.inputSize(), 2);      // Starts at minimum (1 data + 1 select)
    QCOMPARE(elm.outputSize(), 2);     // Starts at minimum (2 outputs for 1-to-2)
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.maxInputSize(), 4);   // Now supports up to 1-to-8 (1 data + 3 select)
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 8);  // Now supports up to 8 outputs
    QCOMPARE(elm.elementType(), ElementType::Demux);
}

// ============================================================================
// Comprehensive Mux Tests
// ============================================================================

void TestMultiplexing::testMuxVariableSizes()
{
    Mux mux;

    // Test default minimum size
    QCOMPARE(mux.inputSize(), 3);
    QCOMPARE(mux.outputSize(), 1);
    QCOMPARE(mux.minInputSize(), 3);
    QCOMPARE(mux.maxInputSize(), 11);

    // Test resizing to different valid sizes
    mux.setInputSize(3);
    QCOMPARE(mux.inputSize(), 3);
    // Verify all ports exist
    for (int i = 0; i < mux.inputSize(); ++i) {
        QVERIFY(mux.inputPort(i) != nullptr);
    }

    mux.setInputSize(6);
    QCOMPARE(mux.inputSize(), 6);
    for (int i = 0; i < mux.inputSize(); ++i) {
        QVERIFY(mux.inputPort(i) != nullptr);
    }

    mux.setInputSize(11);
    QCOMPARE(mux.inputSize(), 11);
    for (int i = 0; i < mux.inputSize(); ++i) {
        QVERIFY(mux.inputPort(i) != nullptr);
    }

    // Test resize beyond max is ignored (stays at current size)
    mux.setInputSize(20);
    QCOMPARE(mux.inputSize(), 11);

    // Test resize below min is ignored (stays at current size)
    mux.setInputSize(1);
    QCOMPARE(mux.inputSize(), 11);  // Stays at 11, not clamped

    // Test invalid negative size is ignored (stays at current size)
    mux.setInputSize(-5);
    QCOMPARE(mux.inputSize(), 11);

    // Test zero size is ignored (stays at current size)
    mux.setInputSize(0);
    QCOMPARE(mux.inputSize(), 11);

    // Test valid resize after invalid attempts still works
    mux.setInputSize(6);
    QCOMPARE(mux.inputSize(), 6);
}

void TestMultiplexing::testMuxPortNaming()
{
    Mux mux;

    // Test port names at minimum size (3 inputs: 2 data + 1 select)
    QCOMPARE(mux.inputPort(0)->name(), QString("In0"));
    QCOMPARE(mux.inputPort(1)->name(), QString("In1"));
    QCOMPARE(mux.inputPort(2)->name(), QString("S0"));
    QCOMPARE(mux.outputPort(0)->name(), QString("Out"));

    // Test port names at 4:1 mux (6 inputs: 4 data + 2 select)
    mux.setInputSize(6);
    for (int i = 0; i < 4; ++i) {
        QCOMPARE(mux.inputPort(i)->name(), QString("In%1").arg(i));
    }
    QCOMPARE(mux.inputPort(4)->name(), QString("S0"));
    QCOMPARE(mux.inputPort(5)->name(), QString("S1"));

    // Test port names at 8:1 mux (11 inputs: 8 data + 3 select)
    mux.setInputSize(11);
    for (int i = 0; i < 8; ++i) {
        QCOMPARE(mux.inputPort(i)->name(), QString("In%1").arg(i));
    }
    QCOMPARE(mux.inputPort(8)->name(), QString("S0"));
    QCOMPARE(mux.inputPort(9)->name(), QString("S1"));
    QCOMPARE(mux.inputPort(10)->name(), QString("S2"));
}

void TestMultiplexing::testMuxEdgeCases()
{
    Mux mux;

    // Test output size is always 1 and cannot be changed
    QCOMPARE(mux.outputSize(), 1);
    mux.setInputSize(6);
    QCOMPARE(mux.outputSize(), 1);
    mux.setInputSize(11);
    QCOMPARE(mux.outputSize(), 1);

    // Test sequential resizing works correctly
    mux.setInputSize(3);
    QCOMPARE(mux.inputSize(), 3);
    mux.setInputSize(6);
    QCOMPARE(mux.inputSize(), 6);
    mux.setInputSize(3);
    QCOMPARE(mux.inputSize(), 3);

    // Test port positioning at minimum size (2:1 mux with 2 data + 1 select)
    auto port0Pos = mux.inputPort(0)->pos();
    QCOMPARE(port0Pos.x(), 16);  // First data input on left
    QVERIFY(port0Pos.y() > 0);   // Y position set

    auto port1Pos = mux.inputPort(1)->pos();
    QCOMPARE(port1Pos.x(), 16);  // Second data input also on left
    QVERIFY(port1Pos.y() != port0Pos.y());  // Different Y from first data

    auto selectPos = mux.inputPort(2)->pos();
    QVERIFY(selectPos.x() >= 32);  // Select line at bottom
    QCOMPARE(selectPos.y(), 56.0);  // All selects at y=56

    auto outputPos = mux.outputPort(0)->pos();
    QCOMPARE(outputPos.x(), 48);  // Output on right
    QCOMPARE(outputPos.y(), 32.0);  // Output centered

    // Test port access bounds checking
    mux.setInputSize(6);
    QVERIFY(mux.inputPort(0) != nullptr);
    QVERIFY(mux.inputPort(5) != nullptr);
    // Accessing invalid port - should not crash (depends on implementation)

    // Test port count consistency after resize
    mux.setInputSize(11);
    int count = 0;
    for (int i = 0; i < 20; ++i) {
        if (mux.inputPort(i) != nullptr) count++;
    }
    QCOMPARE(count, 11);
}

void TestMultiplexing::testMuxBoundaryValues()
{
    Mux mux;

    // Test at minimum size boundary
    QCOMPARE(mux.inputSize(), mux.minInputSize());
    QCOMPARE(mux.inputSize(), 3);

    // Test at maximum size boundary
    mux.setInputSize(mux.maxInputSize());
    QCOMPARE(mux.inputSize(), 11);

    // Test just beyond maximum - invalid sizes are ignored (stay at current size)
    mux.setInputSize(mux.maxInputSize() + 1);
    QCOMPARE(mux.inputSize(), 11);  // Stays at 11 since 12 is invalid

    // Test just below minimum - invalid sizes are ignored (stay at current size)
    mux.setInputSize(mux.minInputSize() - 1);
    QCOMPARE(mux.inputSize(), 11);  // Stays at 11 since 2 is invalid

    // Test all valid intermediate sizes work and maintain port validity
    for (int size = mux.minInputSize(); size <= mux.maxInputSize(); ++size) {
        mux.setInputSize(size);
        QCOMPARE(mux.inputSize(), size);

        // Verify all ports are accessible
        for (int i = 0; i < size; ++i) {
            QVERIFY(mux.inputPort(i) != nullptr);
            QVERIFY(!mux.inputPort(i)->name().isEmpty());
        }
    }

    // Test extreme values don't crash - invalid sizes are ignored
    mux.setInputSize(INT_MAX);
    QCOMPARE(mux.inputSize(), 11);  // Stays at 11 since INT_MAX is invalid
    mux.setInputSize(INT_MIN);
    QCOMPARE(mux.inputSize(), 11);  // Stays at 11 since INT_MIN is invalid
}

// ============================================================================
// Comprehensive Demux Tests
// ============================================================================

void TestMultiplexing::testDemuxVariableSizes()
{
    Demux demux;

    // Test default minimum size (1-to-2)
    QCOMPARE(demux.inputSize(), 2);
    QCOMPARE(demux.outputSize(), 2);
    QCOMPARE(demux.minInputSize(), 2);
    QCOMPARE(demux.maxInputSize(), 4);
    QCOMPARE(demux.minOutputSize(), 2);
    QCOMPARE(demux.maxOutputSize(), 8);

    // Test resizing to 4 outputs (input auto-calculated to 3: 1 data + 2 selects)
    demux.setOutputSize(4);
    QCOMPARE(demux.inputSize(), 3);
    QCOMPARE(demux.outputSize(), 4);
    // Verify all ports exist
    for (int i = 0; i < demux.inputSize(); ++i) {
        QVERIFY(demux.inputPort(i) != nullptr);
    }
    for (int i = 0; i < demux.outputSize(); ++i) {
        QVERIFY(demux.outputPort(i) != nullptr);
    }

    // Test resizing to maximum (8 outputs, input auto-calculated to 4: 1 data + 3 selects)
    demux.setOutputSize(8);
    QCOMPARE(demux.inputSize(), 4);
    QCOMPARE(demux.outputSize(), 8);
    for (int i = 0; i < demux.inputSize(); ++i) {
        QVERIFY(demux.inputPort(i) != nullptr);
    }
    for (int i = 0; i < demux.outputSize(); ++i) {
        QVERIFY(demux.outputPort(i) != nullptr);
    }

    // Test output resize beyond max is ignored (stays at current size)
    demux.setOutputSize(16);
    QCOMPARE(demux.outputSize(), 8);  // Stays at 8 since 16 is invalid
    QCOMPARE(demux.inputSize(), 4);   // Input stays in sync with output

    // Test output resize below min is ignored (stays at current size)
    demux.setOutputSize(1);
    QCOMPARE(demux.outputSize(), 8);  // Stays at 8 since 1 is invalid
    QCOMPARE(demux.inputSize(), 4);   // Input stays in sync with output

    // Test invalid negative and zero output sizes are all ignored
    demux.setOutputSize(-5);
    QCOMPARE(demux.outputSize(), 8);  // Stays at current
    QCOMPARE(demux.inputSize(), 4);   // Input stays in sync
    demux.setOutputSize(0);
    QCOMPARE(demux.outputSize(), 8);  // Stays at current
    QCOMPARE(demux.inputSize(), 4);   // Input stays in sync
}

void TestMultiplexing::testDemuxPortNaming()
{
    Demux demux;

    // Test port names at minimum size (2 inputs, 2 outputs)
    QCOMPARE(demux.inputPort(0)->name(), QString("In"));
    QCOMPARE(demux.inputPort(1)->name(), QString("S0"));
    QCOMPARE(demux.outputPort(0)->name(), QString("Out0"));
    QCOMPARE(demux.outputPort(1)->name(), QString("Out1"));

    // Test port names for 1-to-4 (input auto-calculated: 1 data + 2 select, 4 outputs)
    demux.setOutputSize(4);
    QCOMPARE(demux.inputPort(0)->name(), QString("In"));
    QCOMPARE(demux.inputPort(1)->name(), QString("S0"));
    QCOMPARE(demux.inputPort(2)->name(), QString("S1"));
    for (int i = 0; i < 4; ++i) {
        QCOMPARE(demux.outputPort(i)->name(), QString("Out%1").arg(i));
    }

    // Test port names for 1-to-8 (input auto-calculated: 1 data + 3 select, 8 outputs)
    demux.setOutputSize(8);
    QCOMPARE(demux.inputPort(0)->name(), QString("In"));
    QCOMPARE(demux.inputPort(1)->name(), QString("S0"));
    QCOMPARE(demux.inputPort(2)->name(), QString("S1"));
    QCOMPARE(demux.inputPort(3)->name(), QString("S2"));
    for (int i = 0; i < 8; ++i) {
        QCOMPARE(demux.outputPort(i)->name(), QString("Out%1").arg(i));
    }

    // Test all output ports have correct names across all sizes
    for (int size = demux.minOutputSize(); size <= demux.maxOutputSize(); ++size) {
        // Reset outputs to minimum first, then set to target size
        demux.setOutputSize(demux.minOutputSize());
        demux.setOutputSize(size);
        for (int i = 0; i < size; ++i) {
            QCOMPARE(demux.outputPort(i)->name(), QString("Out%1").arg(i));
        }
    }
}

void TestMultiplexing::testDemuxEdgeCases()
{
    Demux demux;

    // Test single data input constraint
    QCOMPARE(demux.inputSize(), 2);  // 1 data + 1 select minimum

    // Test output sizing auto-calculates input (4 outputs = 1 data + 2 selects)
    demux.setOutputSize(4);
    QCOMPARE(demux.inputSize(), 3);  // 1 data + 2 selects for 4 outputs

    // Test output sizing auto-calculates input (8 outputs = 1 data + 3 selects)
    demux.setOutputSize(8);
    QCOMPARE(demux.inputSize(), 4);  // 1 data + 3 selects for 8 outputs

    // Test multiple output resize attempts (ensure inputs auto-adjust)
    for (int attempt = 0; attempt < 5; ++attempt) {
        int targetOutput = 4 + attempt * 2;
        // Clamp to valid output range
        if (targetOutput > 8) targetOutput = 8;
        demux.setOutputSize(targetOutput);
        QVERIFY(demux.outputSize() >= 2 && demux.outputSize() <= 8);
    }

    // Test sequential resizing
    demux.setOutputSize(2);
    QCOMPARE(demux.inputSize(), 2);
    QCOMPARE(demux.outputSize(), 2);
    demux.setOutputSize(4);
    QCOMPARE(demux.inputSize(), 3);
    QCOMPARE(demux.outputSize(), 4);
    demux.setOutputSize(2);
    QCOMPARE(demux.inputSize(), 2);
    QCOMPARE(demux.outputSize(), 2);

    // Test port positioning at minimum size
    auto dataInputPos = demux.inputPort(0)->pos();
    QCOMPARE(dataInputPos.x(), 16);  // Data input on left
    QVERIFY(dataInputPos.y() > 0);   // Y position set

    auto selectPos = demux.inputPort(1)->pos();
    QVERIFY(selectPos.x() >= 32);  // Select line at bottom
    QVERIFY(selectPos.y() > dataInputPos.y());

    auto firstOutputPos = demux.outputPort(0)->pos();
    QCOMPARE(firstOutputPos.x(), 48);  // Output on right
    QVERIFY(firstOutputPos.y() > 0);

    // Test multiple output positioning at different sizes (input auto-calculated)
    demux.setOutputSize(4);
    for (int i = 0; i < 4; ++i) {
        auto pos = demux.outputPort(i)->pos();
        QCOMPARE(pos.x(), 48);  // All outputs on right side
    }

    // Test port count consistency
    demux.setOutputSize(8);
    int inputCount = 0, outputCount = 0;
    for (int i = 0; i < 20; ++i) {
        if (demux.inputPort(i) != nullptr) inputCount++;
        if (demux.outputPort(i) != nullptr) outputCount++;
    }
    QCOMPARE(inputCount, 4);
    QCOMPARE(outputCount, 8);
}

void TestMultiplexing::testDemuxBoundaryValues()
{
    Demux demux;

    // Test at minimum size boundary
    QCOMPARE(demux.inputSize(), demux.minInputSize());
    QCOMPARE(demux.outputSize(), demux.minOutputSize());
    QCOMPARE(demux.inputSize(), 2);
    QCOMPARE(demux.outputSize(), 2);

    // Test at maximum size boundary
    demux.setOutputSize(demux.maxOutputSize());
    QCOMPARE(demux.inputSize(), 4);
    QCOMPARE(demux.outputSize(), 8);

    // Test just beyond maximum (output only - input is auto-calculated)
    demux.setOutputSize(demux.maxOutputSize() + 1);
    QCOMPARE(demux.inputSize(), 4);
    QCOMPARE(demux.outputSize(), 8);

    // Test just below minimum (output only - input is auto-calculated)
    demux.setOutputSize(demux.minOutputSize() - 1);
    QCOMPARE(demux.inputSize(), 4);   // Stays in sync with output
    QCOMPARE(demux.outputSize(), 8);  // Stays at 8 since 1 is invalid

    // Reset to minimum for next tests
    demux.setOutputSize(demux.minOutputSize());
    QCOMPARE(demux.inputSize(), 2);
    QCOMPARE(demux.outputSize(), 2);

    // Test all valid output sizes and verify ports and input is auto-calculated
    for (int outSize = demux.minOutputSize(); outSize <= demux.maxOutputSize(); ++outSize) {
        demux.setOutputSize(outSize);
        QCOMPARE(demux.outputSize(), outSize);

        // Verify input is auto-calculated correctly
        int expectedInputs = 2;  // Start with 1 data + 1 select for 2 outputs
        while ((1 << (expectedInputs - 1)) < outSize) {
            expectedInputs++;
        }
        QCOMPARE(demux.inputSize(), expectedInputs);

        // Verify all output ports are accessible
        for (int i = 0; i < outSize; ++i) {
            QVERIFY(demux.outputPort(i) != nullptr);
            QVERIFY(!demux.outputPort(i)->name().isEmpty());
        }

        // Verify all input ports are accessible
        for (int i = 0; i < expectedInputs; ++i) {
            QVERIFY(demux.inputPort(i) != nullptr);
        }
    }

    // Test extreme output values don't crash - invalid sizes are ignored
    demux.setOutputSize(INT_MAX);
    QCOMPARE(demux.outputSize(), 8);  // Stays at current since INT_MAX is invalid
    demux.setOutputSize(INT_MIN);
    QCOMPARE(demux.outputSize(), 8);  // Stays at current since INT_MIN is invalid
}

