#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test Fixtures for MCP Test Suite

Circuit specifications, truth tables, and test data for MCP tests.
"""

from typing import Any, Dict, TYPE_CHECKING

# Import for runtime use
from type_models import create_circuit_spec, CircuitSpecification

if TYPE_CHECKING:
    pass  # All imports now done at runtime

# ==================== BASIC LOGIC GATE CIRCUITS ====================
# Basic logic gate circuit definitions

BASIC_AND_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "A"},
        {"id": 2, "type": "InputButton", "x": 100, "y": 200, "label": "B"},
        {"id": 3, "type": "And", "x": 300, "y": 150},
        {"id": 4, "type": "Led", "x": 500, "y": 150, "label": "OUT"},
    ],
    "connections": [
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
        {"source": 3, "target": 4, "source_port": 0, "target_port": 0},
    ],
}

BASIC_OR_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "A"},
        {"id": 2, "type": "InputButton", "x": 100, "y": 200, "label": "B"},
        {"id": 3, "type": "Or", "x": 300, "y": 150},
        {"id": 4, "type": "Led", "x": 500, "y": 150, "label": "OUT"},
    ],
    "connections": [
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
        {"source": 3, "target": 4, "source_port": 0, "target_port": 0},
    ],
}

BASIC_XOR_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "A"},
        {"id": 2, "type": "InputButton", "x": 100, "y": 200, "label": "B"},
        {"id": 3, "type": "Xor", "x": 300, "y": 150},
        {"id": 4, "type": "Led", "x": 500, "y": 150, "label": "OUT"},
    ],
    "connections": [
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
        {"source": 3, "target": 4, "source_port": 0, "target_port": 0},
    ],
}

# ==================== SIMPLE CIRCUITS ====================
# Simple circuit definitions

SIMPLE_INPUT_OUTPUT_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "INPUT"},
        {"id": 2, "type": "Led", "x": 300, "y": 100, "label": "OUT"},
    ],
    "connections": [
        {"source": 1, "target": 2, "source_port": 0, "target_port": 0},
    ],
}

BASIC_NOT_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 0, "y": 50, "label": "INPUT"},
        {"id": 2, "type": "Not", "x": 100, "y": 50, "label": "NOT_GATE"},
        {"id": 3, "type": "Led", "x": 200, "y": 50, "label": "OUTPUT"}
    ],
    "connections": [
        {"source": 1, "target": 2, "source_port": 0, "target_port": 0},
        {"source": 2, "target": 3, "source_port": 0, "target_port": 0}
    ]
}

# ==================== MULTI-SIGNAL CIRCUITS ====================
# Multi-signal circuit definitions

MULTI_SIGNAL_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "A"},
        {"id": 2, "type": "InputButton", "x": 50, "y": 200, "label": "B"},
        {"id": 3, "type": "And", "x": 200, "y": 150, "label": "AND_GATE"},
        {"id": 4, "type": "Or", "x": 200, "y": 250, "label": "OR_GATE"},
        {"id": 5, "type": "Led", "x": 350, "y": 150, "label": "AND_OUT"},
        {"id": 6, "type": "Led", "x": 350, "y": 250, "label": "OR_OUT"},
    ],
    "connections": [
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # A -> AND
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # B -> AND
        {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # A -> OR
        {"source": 2, "target": 4, "source_port": 0, "target_port": 1},  # B -> OR
        {"source": 3, "target": 5, "source_port": 0, "target_port": 0},  # AND -> AND_OUT
        {"source": 4, "target": 6, "source_port": 0, "target_port": 0},  # OR -> OR_OUT
    ],
}

# ==================== SEQUENTIAL CIRCUITS ====================
# Sequential circuit definitions

D_FLIP_FLOP_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "D"},  # Data
        {"id": 2, "type": "InputButton", "x": 50, "y": 200, "label": "CLK"},  # Clock
        {"id": 3, "type": "DFlipFlop", "x": 200, "y": 150, "label": "DFF"},  # D Flip-Flop
        {"id": 4, "type": "Led", "x": 350, "y": 130, "label": "Q"},  # Q output
        {"id": 5, "type": "Led", "x": 350, "y": 170, "label": "Q_NOT"},  # ~Q output
    ],
    "connections": [
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # D -> DFF.D
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # CLK -> DFF.CLK
        {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # DFF.Q -> Q
        {"source": 3, "target": 5, "source_port": 1, "target_port": 0},  # DFF.~Q -> Q_NOT
    ],
}

D_LATCH_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "x": 0, "y": 50, "label": "D"},
        {"id": 2, "type": "InputButton", "x": 0, "y": 100, "label": "EN"},
        {"id": 3, "type": "DLatch", "x": 150, "y": 75, "label": "D_LATCH"},
        {"id": 4, "type": "Led", "x": 300, "y": 60, "label": "Q"},
        {"id": 5, "type": "Led", "x": 300, "y": 90, "label": "Q_NOT"},
    ],
    "connections": [
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # D → DLatch.D
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # EN → DLatch.EN
        {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # DLatch.Q → Q
        {"source": 3, "target": 5, "source_port": 1, "target_port": 0},  # DLatch.~Q → Q_NOT
    ],
}

JK_FLIP_FLOP_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "label": "J", "x": 0, "y": 0},
        {"id": 2, "type": "InputButton", "label": "K", "x": 0, "y": 100},
        {"id": 3, "type": "InputButton", "label": "CLK", "x": 0, "y": 200},
        {"id": 4, "type": "JKFlipFlop", "label": "JK_FF", "x": 200, "y": 100},
        {"id": 5, "type": "Led", "label": "Q", "x": 400, "y": 50},
        {"id": 6, "type": "Led", "label": "Q_NOT", "x": 400, "y": 150},
    ],
    "connections": [
        {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # J -> JK_FF.J (port 0)
        {"source": 2, "target": 4, "source_port": 0, "target_port": 1},  # K -> JK_FF.K (port 1)
        {"source": 3, "target": 4, "source_port": 0, "target_port": 2},  # CLK -> JK_FF.CLK (port 2)
        {"source": 4, "target": 5, "source_port": 0, "target_port": 0},  # JK_FF.Q -> Q
        {"source": 4, "target": 6, "source_port": 1, "target_port": 0},  # JK_FF.~Q -> Q_NOT
    ],
}

T_FLIP_FLOP_CIRCUIT = {
    "elements": [
        {"id": 1, "type": "InputButton", "label": "T", "x": 0, "y": 0},
        {"id": 2, "type": "InputButton", "label": "CLK", "x": 0, "y": 100},
        {"id": 3, "type": "TFlipFlop", "label": "T_FF", "x": 200, "y": 50},
        {"id": 4, "type": "Led", "label": "Q_OUT", "x": 400, "y": 50},
    ],
    "connections": [
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # T -> T_FF.T (port 0)
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # CLK -> T_FF.CLK (port 1)
        {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # T_FF.Q -> Q_OUT
    ],
}

# ==================== COMPLEX CIRCUITS ====================

HALF_ADDER_CIRCUIT = {
    "elements": [
        # Inputs
        {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "A"},
        {"id": 2, "type": "InputButton", "x": 50, "y": 200, "label": "B"},
        # XOR for Sum (A XOR B) - using native XOR gate
        {"id": 3, "type": "Xor", "x": 250, "y": 80, "label": "SUM_XOR"},
        # AND for Carry (A AND B)
        {"id": 4, "type": "And", "x": 250, "y": 180, "label": "CARRY_AND"},
        # Outputs
        {"id": 5, "type": "Led", "x": 450, "y": 80, "label": "SUM"},
        {"id": 6, "type": "Led", "x": 450, "y": 180, "label": "CARRY"},
    ],
    "connections": [
        # Connect inputs to logic gates
        {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # A -> XOR for sum
        {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # B -> XOR for sum
        {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # A -> AND for carry
        {"source": 2, "target": 4, "source_port": 0, "target_port": 1},  # B -> AND for carry
        # Connect gates to outputs
        {"source": 3, "target": 5, "source_port": 0, "target_port": 0},  # SUM_XOR -> SUM
        {"source": 4, "target": 6, "source_port": 0, "target_port": 0},  # CARRY_AND -> CARRY
    ],
}

SR_LATCH_CIRCUIT = {
    "elements": [
        # Inputs
        {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "S"},  # Set
        {"id": 2, "type": "InputButton", "x": 50, "y": 200, "label": "R"},  # Reset
        {"id": 3, "type": "InputButton", "x": 50, "y": 300, "label": "EN"},  # Enable (Clock)
        # Logic gates for SR Latch with Enable
        {"id": 4, "type": "And", "x": 150, "y": 100, "label": "S_AND_EN"},  # S AND EN
        {"id": 5, "type": "And", "x": 150, "y": 200, "label": "R_AND_EN"},  # R AND EN
        {"id": 6, "type": "Nor", "x": 250, "y": 120, "label": "NOR1"},  # First NOR
        {"id": 7, "type": "Nor", "x": 250, "y": 180, "label": "NOR2"},  # Second NOR
        # Outputs
        {"id": 8, "type": "Led", "x": 350, "y": 120, "label": "Q"},
        {"id": 9, "type": "Led", "x": 350, "y": 180, "label": "Q_NOT"},
    ],
    "connections": [
        # Input connections
        {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # S -> S_AND_EN
        {"source": 3, "target": 4, "source_port": 0, "target_port": 1},  # EN -> S_AND_EN
        {"source": 2, "target": 5, "source_port": 0, "target_port": 0},  # R -> R_AND_EN
        {"source": 3, "target": 5, "source_port": 0, "target_port": 1},  # EN -> R_AND_EN
        # Cross-coupling (feedback)
        {"source": 4, "target": 6, "source_port": 0, "target_port": 0},  # S_AND_EN -> NOR1
        {"source": 7, "target": 6, "source_port": 0, "target_port": 1},  # NOR2 -> NOR1 (feedback)
        {"source": 5, "target": 7, "source_port": 0, "target_port": 0},  # R_AND_EN -> NOR2
        {"source": 6, "target": 7, "source_port": 0, "target_port": 1},  # NOR1 -> NOR2 (feedback)
        # Output connections
        {"source": 6, "target": 8, "source_port": 0, "target_port": 0},  # NOR1 -> Q
        {"source": 7, "target": 9, "source_port": 0, "target_port": 0},  # NOR2 -> Q_NOT
    ],
}

# ==================== TRUTH TABLES ====================
# Truth table definitions

AND_TRUTH_TABLE = [
    (False, False, False),
    (False, True, False),
    (True, False, False),
    (True, True, True),
]

OR_TRUTH_TABLE = [
    (False, False, False),
    (False, True, True),
    (True, False, True),
    (True, True, True),
]

XOR_TRUTH_TABLE = [
    (False, False, False),
    (False, True, True),
    (True, False, True),
    (True, True, False),
]

NOT_TRUTH_TABLE = [
    (False, True),
    (True, False),
]

# ==================== TEST DATA SETS ====================
# Performance test data and constants

PERFORMANCE_THRESHOLDS = {
    "new_circuit_time": 2.0,        # Maximum time for circuit creation
    "element_creation_time": 1.0,   # Maximum time for element creation
    "simulation_start_time": 3.0,   # Maximum time to start simulation
    "memory_growth_mb": 50.0,       # Maximum memory growth in MB
    "list_elements_time": 1.0,      # Maximum time to list elements
    "connection_time": 1.0,         # Maximum time for connections
    "critical_path_time": 1.0,      # Maximum critical path timing
}

ELEMENT_TYPES_TO_TEST = [
    # Input elements
    "InputButton", "InputSwitch", "InputRotary", "InputVcc", "InputGnd",
    # Logic gates
    "And", "Or", "Not", "Nand", "Xor", "Xnor", "Node",
    # Output elements
    "Led", "Buzzer", "AudioBox", "Display7", "Display14", "Display16",
    # Sequential elements
    "DFlipFlop", "TFlipFlop", "JKFlipFlop", "SRFlipFlop", "SRLatch", "DLatch",
    # Complex elements
    "Mux", "Demux", "IC", "TruthTable", "Line", "Text", "Clock",
]

# ==================== LARGE CIRCUIT GENERATORS ====================
# Functions to generate large circuit specifications


def create_not_chain_circuit(chain_length: int) -> CircuitSpecification:
    """Generate a circuit with a chain of NOT gates

    Args:
        chain_length: Number of NOT gates in the chain

    Returns:
        Circuit specification dictionary
    """
    elements = [
        {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "INPUT"},
    ]
    connections = []

    # Add NOT gates in chain
    for i in range(chain_length):
        elements.append(
            {
                "id": i + 2,
                "type": "Not",
                "x": 150 + (i * 50),
                "y": 100,
                "label": f"NOT_{i+1}",
            }
        )

        # Connect to previous element
        if i == 0:
            # First NOT connects to input
            connections.append(
                {
                    "source": 1,
                    "target": 2,
                    "source_port": 0,
                    "target_port": 0,
                }
            )
        else:
            # Subsequent NOTs connect to previous NOT
            connections.append(
                {
                    "source": i + 1,
                    "target": i + 2,
                    "source_port": 0,
                    "target_port": 0,
                }
            )

    # Add output LED
    output_id = chain_length + 2
    elements.append(
        {
            "id": output_id,
            "type": "Led",
            "x": 150 + (chain_length * 50),
            "y": 100,
            "label": "OUTPUT",
        }
    )

    # Connect last NOT to output
    connections.append(
        {
            "source": chain_length + 1,
            "target": output_id,
            "source_port": 0,
            "target_port": 0,
        }
    )

    return create_circuit_spec(elements, connections)


# Pre-built large circuits for common test scenarios
LARGE_NOT_CHAIN_25 = create_not_chain_circuit(25)
LARGE_NOT_CHAIN_100 = create_not_chain_circuit(100)
