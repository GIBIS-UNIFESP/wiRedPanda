// Flow definitions: simulation
flowRegistry['sim_ops'] = {
  title: 'Simulation \u2014 Engine',
  nodes: [
    ['root',   'Simulation Engine',                  'start', '1ms timer, 4-phase update, topological sort'],
    ['timer',  'Timer fires\nevery 1ms',             'step',  ''],
    ['d_init', 'Initialized?',                        'decision', 'Lazy: rebuild sim graph on first tick after restart'],
    ['init',   'Initialize',                           'key', 'Collect items, build graph, topological sort', 'sim_init'],
    ['p0',     'Phase 0: Clocks',                     'step',  'Check elapsed time, toggle if needed'],
    ['p1',     'Phase 1: Inputs',                     'step',  'Push switch/button/rotary state'],
    ['p2',     'Phase 2: Logic',                      'key',   'Run all element logic in topological order', 'ge_logic'],
    ['d_fb',   'Has feedback\nloops?',                'decision', ''],
    ['settle', 'Iterative Settle',                    'key', 'Loop up to 10x until converged', 'sim_settle'],
    ['p3',     'Phase 3\u20134: Visuals',             'step',  'Update output + input port visuals'],
    ['wait',   'Wait for\nnext tick',                 'end',   ''],
  ],
  edges: [
    ['root',   'timer'],
    ['timer',  'd_init'],
    ['d_init', 'init',   'No'],
    ['d_init', 'p0',     'Yes'],
    ['init',   'p0'],
    ['p0',     'p1'],
    ['p1',     'p2'],
    ['p2',     'd_fb'],
    ['d_fb',   'settle', 'Yes'],
    ['d_fb',   'p3',     'No'],
    ['settle', 'p3'],
    ['p3',     'wait'],
  ]
};

flowRegistry['sim_init'] = {
  title: 'Initialize Simulation',
  nodes: [
        ['start',   'Initialize Simulation',                    'start',    'Called lazily on first update tick after restart'],
        ['d_scene', 'Scene\navailable?',                        'decision', ''],
        ['r_scene', 'Abort',                                     'error',    ''],
        ['clear',   'Clear all lists:\nclocks, inputs, outputs,\nconnections, sorted', 'step', ''],
        ['items',   'Get scene items\nstable sort by (Y, X)',   'step',    'Deterministic ordering'],
        ['d_one',   'Only 1 item?\n(border rect)',              'decision', ''],
        ['r_one',   'Abort\n(empty scene)',                      'error',    ''],
        ['time',    'Record start time',                         'step',     ''],
        ['cat',     'Categorize items:\nConnections, Clocks,\nInputs, Outputs', 'key', 'Each group stored in a separate list for phased updates'],
        ['d_empty', 'Elements\nempty?',                          'decision', ''],
        ['r_empty', 'Abort',                                     'error',    ''],
        ['init_v',  'Allocate simulation\nvectors per element',  'step',   'Input/output value arrays'],
        ['build_c', 'Build connection graph',                    'step',    'Walk input ports, find source via wire, link predecessors'],
        ['wireless','Connect wireless\nelements',                'step',    'Build Tx label\u2192element map, override Rx predecessors'],
        ['ic_init', 'Initialize IC\nsub-circuits',               'step',   'Recursively build sim graphs for each IC'],
        ['sort',    'Sort elements\nby dependency',              'key',     'Topological sort + feedback detection', 'sim_topo_sort'],
        ['done',    'Ready',                                     'end',      ''],
      ],
  edges: [
        ['start',    'd_scene'],
        ['d_scene',  'r_scene',   'No'],
        ['d_scene',  'clear',     'Yes'],
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

flowRegistry['sim_topo_sort'] = {
  title: 'Topological Sort',
  nodes: [
        ['start',   'Sort Elements\nby Dependency',              'start',    ''],
        ['tx',      'Build wireless\ntransmitter map',           'step',     'Map Tx labels to elements'],
        ['succ',    'Build successor graph',                     'step',    'For each element: who depends on it?'],
        ['topo',    'Topological sort\n(Kahn\u2019s algorithm)', 'key',     ''],
        ['queue',   'Init queue with\nelements having\nno dependencies',  'step',''],
        ['loop',    'Process queue:\npop \u2192 add to sorted\ndecrement successors', 'key', ''],
        ['d_fb',    'Any elements\nnot visited?',                'decision', 'Remaining = feedback nodes'],
        ['no_fb',   'No feedback',                               'step',     ''],
        ['fb',      'Mark feedback nodes',                       'error',    'These elements participate in feedback loops'],
        ['prio',    'Assign priorities\nfrom sorted order',      'step',    'Highest priority = earliest in sorted list'],
        ['done',    'Store sorted elements\nand feedback flags', 'end',''],
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

flowRegistry['sim_settle'] = {
  title: 'Iterative Settle',
  nodes: [
        ['start',   'Iterative Settle\n(max 10 iterations)',     'start',  'Called when feedback loops detected'],
        ['loop',    'For each iteration',                        'key',     ''],
        ['clear',   'Clear output-changed\nflags on all elements', 'step',    ''],
        ['update', 'Run logic\non all elements', 'step', '', 'ge_logic'],
        ['d_conv',  'Any output\nchanged?',                      'decision',''],
        ['conv',    'Converged',                                  'end',     ''],
        ['next',    'Next iteration',                             'step',    ''],
        ['fail',    'Did not converge',                           'error',   'Simulation may emit warning after 10 iterations'],
      ],
  edges: [
        ['start',  'loop'],
        ['loop',   'clear'],
        ['clear',  'update'],
        ['update', 'd_conv'],
        ['d_conv', 'conv',    'No (settled)',  'seg-left'],
        ['d_conv', 'next',    'Yes (changed)', 'seg-right'],
        ['next',   'loop'],
        ['loop',   'fail'],
      ]
};

