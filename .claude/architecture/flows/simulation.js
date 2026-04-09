// Flow definitions: simulation
flowRegistry['simulation_ops'] = {
  title: 'Simulation \u2014 Engine Internals',
  nodes: [
    ['f0', '\u2460 initialize()', 'key', '', 'simulation_u2460_initialize'],
    ['f1', '\u2461 topologicalSort', 'key', '', 'simulation_u2461_topologicalsort'],
    ['f2', '\u2462 iterativeSettle', 'key', '', 'simulation_u2462_iterativesettle']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2']
  ]
};

flowRegistry['simulation_u2460_initialize'] = {
  title: '\u2460 initialize()',
  nodes: [
        ['start',   'initialize()',                          'start',    'Called lazily on first update() tick after restart'],
        ['d_scene', 'm_scene null?',                        'decision', ''],
        ['r_scene', 'return false',                          'error',    ''],
        ['clear',   'Clear all lists:\nclocks, inputs, outputs,\nconnections, sorted', 'step', ''],
        ['items',   'Get scene items\nstable sort by (Y, X)','step',    'Deterministic ordering'],
        ['d_one',   'Only 1 item?\n(border rect)',          'decision', ''],
        ['r_one',   'return false',                          'error',    'Empty scene'],
        ['time',    'globalTime =\nsteady_clock::now()',     'step',     ''],
        ['cat',     'Categorize items:\nConnections \u2192 m_connections\nElements by group', 'key', 'Clock \u2192 m_clocks (resetClock), Input \u2192 m_inputs, Output \u2192 m_outputs'],
        ['d_empty', 'elements\nempty?',                     'decision', ''],
        ['r_empty', 'return false',                          'error',    ''],
        ['init_v',  'initSimulationVectors()\non each element','step',   'Allocate input/output value arrays'],
        ['build_c', 'buildConnectionGraph\n(elements)',      'step',    'Walk input ports \u2192 find source via QNEConnection \u2192 connectPredecessor()'],
        ['wireless','connectWirelessElements\n(elements)',   'step',    'Build Tx label\u2192element map, override Rx predecessors'],
        ['ic_init', 'IC::initializeSimulation()\nfor each IC','step',   'Recursively build sub-circuit sim graphs'],
        ['sort',    'sortSimElements(elements)',              'key',     'Topological sort + feedback detection'],
        ['done',    'm_initialized = true\nreturn true',     'end',      ''],
      ],
  edges: [
        ['start',    'd_scene'],
        ['d_scene',  'r_scene',   'Yes'],
        ['d_scene',  'clear',     'No'],
        ['clear',    'items'],
        ['items',    'd_one'],
        ['d_one',    'r_one',     'Yes'],
        ['d_one',    'time',      'No'],
        ['time',     'cat'],
        ['cat',      'd_empty'],
        ['d_empty',  'r_empty',   'Yes'],
        ['d_empty',  'init_v',    'No'],
        ['init_v',   'build_c'],
        ['build_c',  'wireless'],
        ['wireless', 'ic_init'],
        ['ic_init',  'sort'],
        ['sort',     'done'],
      ]
};

flowRegistry['simulation_u2461_topologicalsort'] = {
  title: '\u2461 topologicalSort',
  nodes: [
        ['start',   'sortSimElements(elements)',             'start',    ''],
        ['tx',      'buildTxMap(elements)',                   'step',     'Map wireless Tx labels to elements'],
        ['succ',    'buildSuccessorGraph\n(elements, txMap)', 'step',    'For each element: who depends on it?'],
        ['topo',    'topologicalSort\n(elements, successors)','key',     'Kahn\u2019s algorithm'],
        ['queue',   'Init queue with\nelements having 0 in-degree','step',''],
        ['loop',    'While queue not empty:\npop \u2192 sorted list\ndecrement successors\u2019 in-degree', 'key', ''],
        ['d_fb',    'Any elements\nnot visited?',            'decision', 'Remaining = feedback nodes'],
        ['no_fb',   'No feedback',                           'step',     ''],
        ['fb',      'Mark as\nm_simFeedbackNodes',          'error',    'These elements participate in feedback loops'],
        ['prio',    'Assign priorities\nfrom sorted order',  'step',    'Highest priority = earliest in sorted list'],
        ['done',    'Store m_sortedElements\nm_simHasFeedbackElements','end',''],
      ],
  edges: [
        ['start',  'tx'],
        ['tx',     'succ'],
        ['succ',   'topo'],
        ['topo',   'queue'],
        ['queue',  'loop'],
        ['loop',   'd_fb'],
        ['d_fb',   'no_fb',  'No'],
        ['d_fb',   'fb',     'Yes'],
        ['no_fb',  'prio'],
        ['fb',     'prio'],
        ['prio',   'done'],
      ]
};

flowRegistry['simulation_u2462_iterativesettle'] = {
  title: '\u2462 iterativeSettle',
  nodes: [
        ['start',   'iterativeSettle\n(elements, maxIter=10)','start',  'Called when feedback loops detected'],
        ['loop',    'iteration = 0\nto maxIter-1',           'key',     ''],
        ['clear',   'clearOutputChanged()\non all elements', 'step',    ''],
        ['update',  'updateLogic()\non all elements',        'step',    ''],
        ['d_conv',  'Any output\nchanged?',                  'decision',''],
        ['conv',    'return true\n(converged)',               'end',     ''],
        ['next',    'Next iteration',                        'step',    ''],
        ['fail',    'return false\n(did not converge)',       'error',   'Simulation may emit warning after 10 iterations'],
      ],
  edges: [
        ['start',  'loop'],
        ['loop',   'clear'],
        ['clear',  'update'],
        ['update', 'd_conv'],
        ['d_conv', 'conv',    'No (settled)'],
        ['d_conv', 'next',    'Yes (changed)'],
        ['next',   'loop'],
        ['loop',   'fail'],
      ]
};

flowRegistry['simulation_mod'] = {
  title: 'Simulation Engine',
  nodes: [
    ['f0', 'Simulation Cycle', 'key', '', 'simulation_simulation_cycle']
  ],
  edges: [

  ]
};

flowRegistry['simulation_simulation_cycle'] = {
  title: 'Simulation Cycle',
  nodes: [
        ['start',      'Simulation::start()',                      'start',    ''],
        ['timer',      'QTimer fires\nevery 1ms',                 'step',     ''],
        ['update',     'update()\n~1000x/sec',                    'key',      'Main simulation loop'],
        ['d_init',     'm_initialized?',                           'decision', 'Lazy init: rebuild sim graph on first tick after restart'],
        ['collect',    'Collect scene items\nsort by (Y, X)',      'step',     'Deterministic ordering. Categorize: m_clocks, m_inputs, m_outputs, m_connections'],
        ['init_vec',   'initSimulationVectors()',                  'step',     'Allocate input/output value arrays on each element'],
        ['build_conn', 'buildConnectionGraph()',                   'step',     'Walk each element input port \u2192 find source via QNEConnection \u2192 connectPredecessor()'],
        ['wireless',   'connectWirelessElements()',                'step',     'Build Tx label\u2192element map, override Rx predecessors'],
        ['ic_init',    'IC::initializeSimulation()',               'step',     'Recursively build sub-circuit sim graphs for each IC element'],
        ['sort',       'sortSimElements()',                        'key',      'Build successor graph \u2192 topologicalSort(). Assigns priority per element. Detects feedback \u2192 m_simFeedbackNodes'],
        ['p0',         'Phase 0: Clocks\nclock\u2192updateClock(t)', 'step',  'Check wall clock elapsed vs interval; if exceeded \u2192 toggle output'],
        ['p1',         'Phase 1: Inputs\ninput\u2192updateOutputs()', 'step', 'Push switch/button/rotary state to sim output slots'],
        ['d_fb',       'Has feedback\nloops?',                     'decision', ''],
        ['single',     'Single pass:\nupdateLogic()\nin topo order', 'step', 'Highest priority first'],
        ['settle',     'iterativeSettle()\nloop up to 10x',       'error',    '1. clearOutputChanged() 2. updateLogic() on all 3. Check if any changed. Converged \u2192 break. Not \u2192 emit warning'],
        ['d_tick',     'Visual tick\ndue?',                        'decision', 'Skip if m_visualTickCount < m_visualTickInterval (throttle to ~60fps)'],
        ['skip_vis',   'Skip repaint',                             'step',     ''],
        ['p3',         'Phase 3:\nupdatePort(output)',             'end',      'Push logic values onto wire visuals'],
        ['p4',         'Phase 4:\nupdatePort(input)',              'end',      'Refresh LED/display/buzzer visuals'],
        ['wait',       'Wait for\nnext tick (1ms)',                'step',     ''],
      ],
  edges: [
        ['start',      'timer'],
        ['timer',      'update'],
        ['update',     'd_init'],
        ['d_init',     'collect',    'No \u2014 initialize()'],
        ['d_init',     'p0',         'Yes \u2014 run phases'],
        ['collect',    'init_vec'],
        ['init_vec',   'build_conn'],
        ['build_conn', 'wireless'],
        ['wireless',   'ic_init'],
        ['ic_init',    'sort'],
        ['sort',       'p0'],
        ['p0',         'p1'],
        ['p1',         'd_fb'],
        ['d_fb',       'single',     'No'],
        ['d_fb',       'settle',     'Yes'],
        ['single',     'd_tick'],
        ['settle',     'd_tick'],
        ['d_tick',     'skip_vis',   'No'],
        ['d_tick',     'p3',         'Yes'],
        ['p3',         'p4'],
        ['skip_vis',   'wait'],
        ['p4',         'wait'],
      ]
};

