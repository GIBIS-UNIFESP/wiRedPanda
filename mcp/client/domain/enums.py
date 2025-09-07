#!/usr/bin/env python3
"""
Domain Enumerations

This module contains enumerations that define the domain concepts
for circuit simulation, element types, and export formats.
"""

from enum import Enum


class SimulationAction(str, Enum):
    """Valid simulation control actions"""

    START = "start"
    STOP = "stop"
    RESTART = "restart"
    UPDATE = "update"


class ElementType(str, Enum):
    """Valid element types for circuit creation"""

    # Logic Gates
    AND = "And"
    OR = "Or"
    NOT = "Not"
    NOR = "Nor"
    NAND = "Nand"
    XOR = "Xor"
    XNOR = "Xnor"
    NODE = "Node"

    # Inputs
    INPUT_BUTTON = "InputButton"
    INPUT_SWITCH = "InputSwitch"
    INPUT_ROTARY = "InputRotary"
    INPUT_VCC = "InputVcc"
    INPUT_GND = "InputGnd"
    CLOCK = "Clock"

    # Outputs
    LED = "Led"
    BUZZER = "Buzzer"
    AUDIO_BOX = "AudioBox"
    DISPLAY_7 = "Display7"
    DISPLAY_14 = "Display14"
    DISPLAY_16 = "Display16"

    # Memory/Sequential Logic
    DFLIPFLOP = "DFlipFlop"
    DLATCH = "DLatch"
    JKFLIPFLOP = "JKFlipFlop"
    SRFLIPFLOP = "SRFlipFlop"
    SRLATCH = "SRLatch"
    TFLIPFLOP = "TFlipFlop"

    # Multiplexers
    MUX = "Mux"
    DEMUX = "Demux"

    # Special Elements
    IC = "IC"
    TRUTH_TABLE = "TruthTable"
    LINE = "Line"
    TEXT = "Text"


class ImageExportFormat(str, Enum):
    """Valid export formats for circuit images"""

    PNG = "png"
    SVG = "svg"
    PDF = "pdf"


class WaveformExportFormat(str, Enum):
    """Valid export formats for waveform data and charts"""

    TXT = "txt"
    PNG = "png"
