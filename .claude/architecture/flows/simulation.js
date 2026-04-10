// Flow definitions: simulation
flowRegistry['sim_ops'] = {
  title: 'Simulation \u2014 Engine',
  nodes: [
    ['root',   'Simulation Engine',                  'start', '1ms timer, 4-phase update, topological sort'],
    ['timer',  'Timer fires\nevery 1ms',             'step',  ''],
    ['d_init', 'Initialized?',                        'decision', 'Lazy: rebuild sim graph on first tick after restart'],
    ['init',   'Initialize',                           'key', 'Collect items, build graph, topological sort', 'sim_init'],
    ['p0',     'Phase 0: Clocks',                     'key',   'Check elapsed time, toggle if needed', 'sim_p0'],
    ['p1',     'Phase 1: Inputs',                     'key',   'Push switch/button/rotary state', 'sim_p1'],
    ['p2',     'Phase 2: Logic',                      'key',   'Single pass or iterative settle if feedback', 'sim_p2'],
    ['p3',     'Phase 3: Wire\nVisuals',              'key',   'Push logic values onto wires', 'sim_p3'],
    ['p4',     'Phase 4: Output\nVisuals',            'key',   'Refresh LEDs, displays, buzzers', 'sim_p4'],
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
    ['p2',     'p3'],
    ['p3',     'p4'],
    ['p4',     'wait'],
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

flowRegistry['sim_p0'] = {
  title: 'Phase 0 \u2014 Clocks',
  nodes: [
    ['start',  'Phase 0: Update Clocks',               'start', 'First phase of each simulation tick'],
    ['loop',   'For each clock element',                'key',   ''],
    ['time',   'Check wall-clock\nelapsed time',        'step',  'Compare current time against clock\u2019s configured interval'],
    ['d_due',  'Interval\nelapsed?',                    'decision', ''],
    ['skip',   'No change\n(keep current state)',       'step',   ''],
    ['toggle', 'Toggle output\n(high \u2194 low)',      'key',   'Flip the clock\u2019s output value'],
    ['reset',  'Reset elapsed\ntime counter',           'step',  ''],
    ['next',   'Next clock',                            'step',   ''],
    ['done',   'All clocks updated',                    'end',   ''],
  ],
  edges: [
    ['start',  'loop'],
    ['loop',   'time'],
    ['time',   'd_due'],
    ['d_due',  'skip',   'No'],
    ['d_due',  'toggle', 'Yes'],
    ['toggle', 'reset'],
    ['skip',   'next'],
    ['reset',  'next'],
    ['next',   'loop'],
    ['loop',   'done'],
  ]
};

flowRegistry['sim_p1'] = {
  title: 'Phase 1 \u2014 Inputs',
  nodes: [
    ['start',  'Phase 1: Update Inputs',               'start', 'Propagate user-controlled input states'],
    ['loop',   'For each input element',                'key',   'Buttons, switches, rotary knobs'],
    ['read',   'Read current\nUI state',                'step',  'Is the button pressed? Switch toggled? Rotary position?'],
    ['push',   'Push state to\nsimulation outputs',     'key',   'Write the input\u2019s value to its output port(s)'],
    ['next',   'Next input',                            'step',   ''],
    ['done',   'All inputs updated',                    'end',   ''],
  ],
  edges: [
    ['start', 'loop'],
    ['loop',  'read'],
    ['read',  'push'],
    ['push',  'next'],
    ['next',  'loop'],
    ['loop',  'done'],
  ]
};

flowRegistry['sim_p2'] = {
  title: 'Phase 2 \u2014 Logic',
  nodes: [
    ['start',  'Phase 2: Update Logic',                'start', 'Core computation phase'],
    ['d_fb',   'Has feedback\nloops?',                  'decision', 'Detected during initialization'],
    ['single', 'Single pass\nin topological order',     'key',   'No feedback: each element computed once, inputs guaranteed ready'],
    ['settle', 'Iterative settle\n(up to 10 passes)',   'key',   'Feedback: repeat until outputs converge', 'sim_settle'],
    ['loop',   'For each element\n(highest priority first)', 'key', ''],
    ['snap',   'Snapshot inputs\nfrom predecessors',    'step', 'Copy predecessor output values into input array', 'ge_logic'],
    ['compute','Compute element logic\n(AND, FF, etc.)','step', 'Each element type overrides this with its own boolean/sequential logic'],
    ['output', 'Set output values\nand change flag',    'step',  ''],
    ['next',   'Next element',                          'step',   ''],
    ['done',   'All logic updated',                     'end',   ''],
  ],
  edges: [
    ['start',   'd_fb'],
    ['d_fb',    'single',  'No'],
    ['d_fb',    'settle',  'Yes'],
    ['single',  'loop'],
    ['settle',  'loop'],
    ['loop',    'snap'],
    ['snap',    'compute'],
    ['compute', 'output'],
    ['output',  'next'],
    ['next',    'loop'],
    ['loop',    'done'],
  ]
};

flowRegistry['sim_p3'] = {
  title: 'Phase 3 \u2014 Wire Visuals',
  nodes: [
    ['start',  'Phase 3: Update\nWire Visuals',        'start', 'Push computed values onto visual wires'],
    ['d_tick', 'Visual tick\ndue?',                     'decision', 'Throttle repaints to ~60fps'],
    ['skip',   'Skip repaint',                          'end',   'Logic still ran, just skip visual update'],
    ['loop',   'For each connection',                   'key',   ''],
    ['read',   'Read element\u2019s\noutput value',     'step',  'Get the computed logic value from the element'],
    ['set',    'Set output port\nstatus + color',       'step',  'Wire turns green (active), red (inactive), or gray (undefined)'],
    ['next',   'Next connection',                       'step',   ''],
    ['done',   'All wires updated',                     'end',   ''],
  ],
  edges: [
    ['start', 'd_tick'],
    ['d_tick', 'skip',  'No'],
    ['d_tick', 'loop',  'Yes'],
    ['loop',  'read'],
    ['read',  'set'],
    ['set',   'next'],
    ['next',  'loop'],
    ['loop',  'done'],
  ]
};

flowRegistry['sim_p4'] = {
  title: 'Phase 4 \u2014 Output Visuals',
  nodes: [
    ['start',  'Phase 4: Update\nOutput Visuals',      'start', 'Refresh output element displays'],
    ['loop',   'For each output element',               'key',   'LEDs, 7-segment displays, buzzers'],
    ['ports',  'For each input port\non the element',   'step',  'Output elements read values from their input ports'],
    ['read',   'Read connected\nwire status',           'step',  'Get the value from the connected output port'],
    ['set',    'Set input port status',                 'step',  'Port color updates to reflect signal state'],
    ['refresh','Element refreshes\nits visual',         'key',   'LED changes color, display shows digit, buzzer toggles sound'],
    ['next',   'Next output element',                   'step',   ''],
    ['done',   'All outputs updated',                   'end',   ''],
  ],
  edges: [
    ['start',   'loop'],
    ['loop',    'ports'],
    ['ports',   'read'],
    ['read',    'set'],
    ['set',     'refresh'],
    ['refresh', 'next'],
    ['next',    'loop'],
    ['loop',    'done'],
  ]
};

