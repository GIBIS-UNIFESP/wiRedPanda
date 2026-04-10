// Flow definitions: test suite

// ── Test Suite Overview ──────────────────────────────────────
flowRegistry['test_ops'] = {
  title: 'Test Suite — Overview',
  nodes: [
    ['root',    'Test Suite\n(132 test classes)',         'start', 'Single binary, runs via ctest --preset debug'],
    ['runner',  'Test Runner',                            'key',   'Filters by class name, passes remaining args to Qt Test', 'test_runner'],
    ['utils',   'Test Utilities\n(CircuitBuilder)',       'key',   'Workspace/scene creation, multi-bit I/O, clock helpers', 'test_utils'],
    ['unit',    'Unit Tests\n(10 categories, ~50 classes)', 'key', 'Isolated component tests — no file I/O, no cross-module deps', 'test_unit_ops'],
    ['integ',   'Integration Tests\n(~80 classes)',       'key',   'Cross-module: IC hierarchy, file load/save, codegen export', 'test_integ_ops'],
    ['system',  'System Tests\n(TestWaveform)',           'key',   'End-to-end BeWavedDolphin waveform simulation', 'test_system_ops'],
    ['compat',  'Backward Compat\n(18 versions)',         'key',   'Load .panda files from v1.8-beta through v4.6.0', 'test_compat_ops'],
  ],
  edges: [
    ['root',   'runner'],
    ['root',   'utils'],
    ['runner', 'unit'],
    ['runner', 'integ'],
    ['runner', 'system'],
    ['runner', 'compat'],
  ]
};

// ── Test Runner ──────────────────────────────────────────────
flowRegistry['test_runner'] = {
  title: 'Test Runner',
  nodes: [
    ['start',    'Run test suite\n(argc, argv, tests)', 'start',    'Entry point'],
    ['env',      'Set up test environment,\nconfigure app',  'step', 'Headless mode on Linux, register metatypes'],
    ['app',      'Create Qt Application',                    'step', 'Full event loop for widget tests'],
    ['d_func',   'List functions\nflag?',                    'decision', ''],
    ['list',     'List all test methods\nby class',          'end', 'Uses Qt meta-object introspection'],
    ['d_class',  'Arg matches\na test class?',               'decision', ''],
    ['single',   'Run single class',                         'key',  'Passes remaining args to Qt Test (e.g. -v2)'],
    ['d_flag',   'Arg starts\nwith "-"?',                    'decision', ''],
    ['err',      'Unknown test class\n(print available, exit 1)', 'error', ''],
    ['all',      'Run ALL classes\nin registration order',   'key',   'Combine exit status for each'],
    ['done',     'Return combined status\n(0 = all pass)',   'end',   ''],
  ],
  edges: [
    ['start',   'env'],
    ['env',     'app'],
    ['app',     'd_func'],
    ['d_func',  'list',    'Yes'],
    ['d_func',  'd_class', 'No'],
    ['d_class', 'single',  'Yes'],
    ['d_class', 'd_flag',  'No'],
    ['d_flag',  'all',     'Yes (Qt Test flag)'],
    ['d_flag',  'err',     'No'],
    ['single',  'done'],
    ['all',     'done'],
  ]
};

// ── Test Utilities ───────────────────────────────────────────
flowRegistry['test_utils'] = {
  title: 'Test Utilities & Circuit Builder',
  nodes: [
    ['root',     'Test Infrastructure',            'start',    'Common helpers in Tests/Common/'],
    ['ws',       'Create workspace',                'step',    'Fresh workspace with initialized scene'],
    ['sw',       'Create switches (batch)',          'step', ''],
    ['multi_in', 'Set multi-bit input\n(decompose int \u2192 bits)', 'step', ''],
    ['multi_out','Read multi-bit output\n(combine bits \u2192 int)', 'step', ''],
    ['clk',      'Clock cycle / toggle\nhelpers',   'step', 'Full pulse (0\u21921\u21920) or single edge'],
    ['status',   'Port status helpers\n(bool checks)', 'step', ''],
    ['builder',  'Circuit Builder',                'key',      'High-level circuit assembly API'],
    ['add',      'Add elements\n(variadic)',        'step', ''],
    ['conn',     'Connect elements\n(by index or label)', 'step', 'Wire elements — supports port labels for ICs'],
    ['sim',      'Initialize simulation\nand return handle', 'step', ''],
  ],
  edges: [
    ['root',     'ws'],
    ['root',     'sw'],
    ['sw',       'multi_in'],
    ['root',     'multi_out'],
    ['root',     'clk'],
    ['root',     'status'],
    ['root',     'builder'],
    ['builder',  'add'],
    ['builder',  'conn'],
    ['builder',  'sim'],
  ]
};

// ── Unit Tests ───────────────────────────────────────────────
flowRegistry['test_unit_ops'] = {
  title: 'Unit Tests — By Category',
  nodes: [
    ['root',     'Unit Tests\nTests/Unit/',                'start', '10 subdirectories, isolated component tests'],
    ['cmd',      'Commands\nTestCommands',                 'step',  'All 10 undo command subclasses: add, delete, move, update, morph, split, flip, rotate, paste, trigger', 'cmd_ops'],
    ['common',   'Common\n6 classes',                      'key',   'TestCommon, TestEnums, TestPriorities, TestRecentFiles, TestSettings, TestThemeManager', 'test_unit_common'],
    ['elements', 'Elements\n18 classes',                   'key',   'Gates, displays, clocks, inputs, geometry, labels, properties, sequential logic, mux, truth table, wireless', 'test_unit_elements'],
    ['factory',  'Factory\nTestElementFactory',            'step',  'Create by type string, metadata registration, round-trip', 'ef_ops'],
    ['logic',    'Logic\n4 classes',                       'key',   'Element logic, error cases, node logic, status ops', 'test_unit_logic'],
    ['nodes',    'Nodes\n2 classes',                       'step',  'TestConnections, TestConnectionSerialization', 'conn_ops'],
    ['scene',    'Scene\n5 classes',                       'key',   'TestScene, TestSceneState, TestSceneConnections, TestSceneUndoredo, TestConnectionValidity', 'test_unit_scene'],
    ['serial',   'Serialization\nTestSerialization',       'step',  'Save/load round-trip, version compat, element fidelity', 'ser_ops'],
    ['sim',      'Simulation\nTestSimulationBlocker',      'step',  'RAII pause/resume guard correctness', 'sim_ops'],
    ['ui',       'UI\nTestDialogs',                        'step',  'ClockDialog, LengthDialog validation'],
  ],
  edges: [
    ['root', 'cmd'],
    ['root', 'common'],
    ['root', 'elements'],
    ['root', 'factory'],
    ['root', 'logic'],
    ['root', 'nodes'],
    ['root', 'scene'],
    ['root', 'serial'],
    ['root', 'sim'],
    ['root', 'ui'],
  ]
};

flowRegistry['test_unit_common'] = {
  title: 'Unit — Common Tests',
  nodes: [
    ['root',     'Common Tests\nTests/Unit/Common/', 'start', '6 test classes for shared utilities'],
    ['common',   'TestCommon\nhelper functions',       'step',  'String utils, path helpers, type conversions'],
    ['enums',    'TestEnums\nElementType coverage',    'step',  'All ElementType/ElementGroup/InputType values'],
    ['prio',     'TestPriorities\nupdate ordering',    'step',  'Priority constants, topological sort values'],
    ['recent',   'TestRecentFiles\nMRU list',          'step',  'Add, remove, max size, persistence'],
    ['settings', 'TestSettings\nconfig wrapper',       'step',  'Read/write config values, defaults'],
    ['theme',    'TestThemeManager\ntheme switching',   'step',  'Light/dark toggle, icon path resolution'],
  ],
  edges: [
    ['root', 'common'],
    ['root', 'enums'],
    ['root', 'prio'],
    ['root', 'recent'],
    ['root', 'settings'],
    ['root', 'theme'],
  ]
};

flowRegistry['test_unit_elements'] = {
  title: 'Unit — Element Tests',
  nodes: [
    ['root',     'Element Tests\nTests/Unit/Elements/',  'start', '18 test classes for circuit components'],
    ['gates',    'TestLogicGates\nAND/OR/NOT/XOR/...',   'step',  'Truth tables for all gate types, fan-in variations'],
    ['seq',      'TestSequentialLogic\nFF & Latches',     'step',  'D/JK/T flip-flops, SR latch edge behavior'],
    ['clk',      'TestClock +\nTestClocksAdvanced',       'step',  'Frequency, duty cycle, toggle timing'],
    ['mux',      'TestMultiplexing\nMux/Demux',           'step',  'Select lines, data routing, all widths'],
    ['disp',     'TestDisplays\n7-Segment, LED arrays',   'step',  'Segment mapping, multi-digit output'],
    ['input',    'TestInputElements +\nTestInputRotary',  'step',  'Button, Switch, Rotary knob value ranges'],
    ['geom',     'TestGeometry\nposition & bounds',       'step',  'Bounding rects, rotation, port positioning'],
    ['label',    'TestElementLabel\ntext labels',          'step',  'Create, move, font, visibility'],
    ['props',    'TestElementProperties\ndynamic props',   'step',  'Get/set, serialization, validation'],
    ['feat',     'TestFeatures\nelement capabilities',     'step',  'Resizable, rotatable, label-editable flags'],
    ['tt',       'TestTruthTable\ntruth table element',    'step',  'Parse, evaluate, custom logic'],
    ['audio',    'TestAudioBox +\nTestBuzzer',             'step',  'Audio element helper functions'],
    ['led',      'TestLED\nLED element',                   'step',  'Color mapping, brightness states'],
    ['comp',     'TestComponents\nall-types smoke test',   'step',  'Create every ElementType, verify defaults'],
    ['wireless', 'TestWirelessNode\nTx/Rx pairing',        'step',  'Label matching, signal propagation'],
    ['adv',      'TestGraphicelementAdvanced\nbase class', 'step',  'Advanced GraphicElement API tests'],
  ],
  edges: [
    ['root', 'gates'],
    ['root', 'seq'],
    ['root', 'clk'],
    ['root', 'mux'],
    ['root', 'disp'],
    ['root', 'input'],
    ['root', 'geom'],
    ['root', 'label'],
    ['root', 'props'],
    ['root', 'feat'],
    ['root', 'tt'],
    ['root', 'audio'],
    ['root', 'led'],
    ['root', 'comp'],
    ['root', 'wireless'],
    ['root', 'adv'],
  ]
};

flowRegistry['test_unit_logic'] = {
  title: 'Unit — Logic Tests',
  nodes: [
    ['root',      'Logic Tests\nTests/Unit/Logic/',    'start', '4 test classes for simulation logic'],
    ['elem_logic','TestElementLogic\ngate evaluation',  'step',  'Combinational logic: all gates produce correct output for all input combos'],
    ['errors',    'TestElementLogicErrors\nerror cases', 'step', 'Undefined inputs, disconnected ports, tristate handling'],
    ['node_logic','TestNodeLogic\nport propagation',    'step',  'Signal flow through port and connection chains'],
    ['status',    'TestStatusOps\ntristate helpers',     'step',  'Active/Inactive/Undefined operations, truth table'],
  ],
  edges: [
    ['root', 'elem_logic'],
    ['root', 'errors'],
    ['root', 'node_logic'],
    ['root', 'status'],
  ]
};

flowRegistry['test_unit_scene'] = {
  title: 'Unit — Scene Tests',
  nodes: [
    ['root',     'Scene Tests\nTests/Unit/Scene/',       'start', '5 test classes for scene operations'],
    ['scene',    'TestScene\ncore scene ops',              'step',  'Add/remove items, selection, clear, item count'],
    ['state',    'TestSceneState\nscene state tracking',   'step',  'Modified flag, dirty detection, state transitions'],
    ['conn',     'TestSceneConnections\nwire management',  'step',  'Create/delete wires via scene API, port validation'],
    ['undo',     'TestSceneUndoredo\nundo stack',          'step',  'Multi-step undo/redo, stack limits, command merging', 'cmd_ops'],
    ['valid',    'TestConnectionValidity\nwire rules',     'step',  'Output\u2192Input only, no self-loops, no duplicate wires', 'conn_validate'],
  ],
  edges: [
    ['root', 'scene'],
    ['root', 'state'],
    ['root', 'conn'],
    ['root', 'undo'],
    ['root', 'valid'],
  ]
};

// ── Integration Tests ────────────────────────────────────────
flowRegistry['test_integ_ops'] = {
  title: 'Integration Tests — By Area',
  nodes: [
    ['root',     'Integration Tests\nTests/Integration/',       'start', 'Cross-module tests with file I/O and full scenes'],
    ['files',    'TestFiles\n.panda file loading',               'step',  'Load all Examples/*.panda, verify element counts, simulate'],
    ['ws',       'TestWorkspace +\nTestWorkspaceFileops',        'step',  'Tab management, save/load round-trip, file operations', 'ws_ops'],
    ['ic',       'TestIC + TestICInline\nIC sub-circuits',       'key',   'File-based ICs, inline embed/extract, nested ICs', 'test_integ_ic'],
    ['sim',      'TestSimulation\nfull-circuit sim',             'step',  'Multi-element circuits, feedback loops, settle behavior', 'sim_ops'],
    ['feedback', 'TestFeedback\nfeedback detection',             'step',  'Circuits with intentional loops, convergence verification'],
    ['arduino',  'TestArduino\nArduino code export',             'step',  'Generate C++ code, compare against expected output', 'cg_arduino'],
    ['verilog',  'TestSystemVerilogExport\nVerilog export',      'step',  'Generate SV code, compare against expected output', 'cg_verilog'],
    ['logic',    'TestMuxDemuxComprehensive\nMux/Demux logic',   'step',  'All mux/demux sizes with exhaustive input patterns'],
    ['cpu',      'IC Hierarchy Tests\n(Levels 1-9, ~70 classes)', 'key',  'Progressive CPU build: latches \u2192 ALU \u2192 register file \u2192 full CPU', 'test_integ_cpu'],
    ['no_panda', 'Tests Without .panda\n4 classes',               'step', 'Decoder8to256, MemorySettlingTime, RAMCell1bit, Sequential'],
  ],
  edges: [
    ['root', 'files'],
    ['root', 'ws'],
    ['root', 'ic'],
    ['root', 'sim'],
    ['root', 'feedback'],
    ['root', 'arduino'],
    ['root', 'verilog'],
    ['root', 'logic'],
    ['root', 'cpu'],
    ['root', 'no_panda'],
  ]
};

flowRegistry['test_integ_ic'] = {
  title: 'Integration — IC Tests',
  nodes: [
    ['root',    'IC Integration Tests',              'start',    'TestIC + TestICInline'],
    ['load',    'TestIC\nLoad .panda as IC',          'step',    'File-based IC loading, port mapping'],
    ['nested',  'Nested ICs\nIC inside IC',           'step',    'Multi-level nesting, recursive simulation'],
    ['sim',     'IC Simulation\nlogic chain',          'step',   'Internal element sort, boundary propagation', 'ic_ops'],
    ['inline',  'TestICInline\nEmbed/Extract',         'key',    'Convert file IC \u2194 inline blob'],
    ['embed',   'Embed IC\nfile \u2192 inline blob',   'step',   'Serialize sub-circuit into parent .panda', 'ic_embed'],
    ['extract', 'Extract IC\ninline blob \u2192 file',  'step',   'Write embedded blob back to .panda file', 'ic_extract'],
    ['self_c',  'Self-contained\nresolve all deps',    'step',   'Recursively embed all nested file ICs', 'icreg_self_contained'],
  ],
  edges: [
    ['root',   'load'],
    ['root',   'inline'],
    ['load',   'nested'],
    ['load',   'sim'],
    ['inline', 'embed'],
    ['inline', 'extract'],
    ['inline', 'self_c'],
  ]
};

flowRegistry['test_integ_cpu'] = {
  title: 'Integration — IC Hierarchy (CPU Build)',
  nodes: [
    ['root',   'IC Hierarchy Tests\nLevels 1-9',         'start', '~70 test classes building a CPU from primitives'],
    ['l1',     'Level 1 — Basics\nDFF, DLatch, JKFF, SR', 'step', 'Fundamental sequential elements as .panda ICs'],
    ['l2',     'Level 2 — Combinational\nDecoders, Adders, Muxes', 'step', 'Decoder2to4/3to8/4to16, FullAdder, HalfAdder, Mux2/4/8, Parity, Priority'],
    ['l3',     'Level 3 — Functional\nALU selector, BCD, Comparator, Register', 'step', 'ALUSelector5way, BCD7Seg, Comparator4bit, Register1bit'],
    ['l4',     'Level 4 — Subsystems\nCounters, RAM, Registers, Shifters', 'step', 'BinaryCounter, BusMux, JohnsonCounter, RAM4x1/8x1, Register4bit, RippleAdder/ALU, ShiftReg'],
    ['l5',     'Level 5 — Controllers\nBarrel shifters, Counters, RegFiles', 'step', 'BarrelRotator/Shifter, ClockGatedDecoder, Controller4bit, InstrDecoder, LoadableCounter, RegFile4x4/8x8'],
    ['l6',     'Level 6 — 8-bit\nALU, RAM, Registers, Stack', 'step', 'ALU8bit, ProgramCounter8bit, RAM256x8, Register8bit, RegFile8x8, RippleAdder8bit, StackPointer'],
    ['l7',     'Level 7 — Datapath\nALU16bit, Forwarding, Flags', 'step', 'ALU16bit, CPUProgCounter, DataForwarding, ExecutionDatapath, FlagReg, InstrDecoder8bit, InstrMemIface, InstrReg8bit'],
    ['l8',     'Level 8 — Pipeline Stages\nFetch, Decode, Execute, Memory', 'step', 'Individual pipeline stage verification'],
    ['l9',     'Level 9 — Full CPUs\n8-bit + 16-bit RISC',  'key',  'SingleCycleCPU8bit, MultiCycleCPU8bit, CPU16bitRISC, FetchStage16bit'],
  ],
  edges: [
    ['root', 'l1'],
    ['l1',   'l2'],
    ['l2',   'l3'],
    ['l3',   'l4'],
    ['l4',   'l5'],
    ['l5',   'l6'],
    ['l6',   'l7'],
    ['l7',   'l8'],
    ['l8',   'l9'],
  ]
};

// ── System Tests ─────────────────────────────────────────────
flowRegistry['test_system_ops'] = {
  title: 'System Tests — BeWavedDolphin',
  nodes: [
    ['root',    'System Tests\nTests/System/',     'start',    'End-to-end waveform simulation tests'],
    ['wf',      'TestWaveform',                     'key',     'Full BeWavedDolphin pipeline test'],
    ['setup',   'Load circuit\n+ configure inputs',  'step',   'Open .panda file, set signal model inputs'],
    ['run',     'Run simulation\nfor N clock cycles', 'step',  'Drive simulation, capture output waveforms', 'bwd_run'],
    ['verify',  'Verify waveform\noutput values',     'step',  'Compare captured signals against expected patterns'],
    ['export',  'Export results\nsave/load round-trip', 'step', 'Serialize waveform data, verify fidelity', 'bwd_save_load'],
  ],
  edges: [
    ['root',   'wf'],
    ['wf',     'setup'],
    ['setup',  'run'],
    ['run',    'verify'],
    ['verify', 'export'],
  ]
};

// ── Backward Compatibility Tests ─────────────────────────────
flowRegistry['test_compat_ops'] = {
  title: 'Backward Compatibility — File Format Versions',
  nodes: [
    ['root',     'Backward Compat Tests', 'start', 'Load .panda files from every released version'],
    ['dir',      'BackwardCompatibility/\n18 version dirs',   'key',   'v1.8-beta through v4.6.0'],
    ['d_load',   'Load each .panda\nvia serialization',       'step',  'Deserialize with version-aware context', 'ser_deserialize'],
    ['d_ver',    'Version detected?',                         'decision', ''],
    ['err',      'Load failure',                               'error',    'Unsupported or corrupt file'],
    ['verify',   'Verify element\ncounts & types',             'step',    'Check expected element types were created'],
    ['sim',      'Run simulation,\nverify logic output',       'step',    'Ensure old circuits still compute correctly'],
    ['pass',     'All versions pass',                          'end',     ''],
    ['v_old',    'Legacy: v1.8\u20132.1\n(3 dirs, 21 files)',  'step',   'Pre-Qt6 format, basic circuits'],
    ['v_mid',    'Middle: v2.3\u2013v2.5\n(3 dirs)',           'step',   'IC support, display elements'],
    ['v_rc',     'Release candidates:\nv4.0.0-rc to rc4',      'step',   'Format stabilization, embedded ICs'],
    ['v_rel',    'Releases: v4.1.9\u2013v4.6.0\n(7 dirs)',    'step',   'Current format lineage: wireless, notes, nested ICs'],
  ],
  edges: [
    ['root',    'dir'],
    ['dir',     'v_old'],
    ['dir',     'v_mid'],
    ['dir',     'v_rc'],
    ['dir',     'v_rel'],
    ['v_old',   'd_load'],
    ['v_mid',   'd_load'],
    ['v_rc',    'd_load'],
    ['v_rel',   'd_load'],
    ['d_load',  'd_ver'],
    ['d_ver',   'err',    'No'],
    ['d_ver',   'verify', 'Yes'],
    ['verify',  'sim'],
    ['sim',     'pass'],
  ]
};
