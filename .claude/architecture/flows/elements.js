// Flow definitions: elements
flowRegistry['ge_ops'] = {
  title: 'GraphicElement \u2014 Element System',
  nodes: [
    ['root',    'Element System',                    'start', ''],
    ['factory', 'Element Factory', 'key', 'Create elements by type', 'ef_build'],
    ['base',    'GraphicElement\n(Base Class)',       'key', 'QGraphicsObject + ItemWithId: ports, pixmap, label, theme'],
    ['save',    'Save Element',                       'key', 'Type \u2192 pos \u2192 rotation \u2192 ports \u2192 props', 'ge_save'],
    ['load',    'Load Element',                       'key', 'Version-aware, registers ports for reconnection', 'ge_load'],
    ['logic',   'Update Logic',                       'key', 'Simulation cycle: snap inputs \u2192 compute \u2192 set outputs', 'ge_logic'],
    ['ports',   'Port Management', 'key', 'Add/remove ports, redistribute vertically', 'ge_ports'],
    ['editor',  'Element Editor\n(Property Panel)',    'key', 'Edit props \u2192 push update command', 'editor_apply'],
    ['reg',     'Static Registration', 'key', 'Auto-registers 40+ types at startup', 'ef_registration'],
  ],
  edges: [
    ['root',    'factory'],
    ['factory', 'base'],
    ['base',    'save'],
    ['base',    'load'],
    ['base',    'logic'],
    ['base',    'ports'],
    ['base',    'editor'],
    ['factory', 'reg'],
  ]
};

flowRegistry['ge_save'] = {
  title: 'Save Element',
  nodes: [
        ['start',  'Save element\nto stream',                  'start',   ''],
        ['type',   'Write element type',                        'step',    ''],
        ['pos',    'Write position,\nrotation, flip',           'step',    ''],
        ['ports',  'Write input/output\nport counts',           'step',    ''],
        ['props',  'Write element-specific\nproperties',        'key',     'Label, color, frequency, delay, trigger, appearances'],
        ['pids',   'Write port serial IDs\nfor reconnection',   'end',    '(elementId << 16) | portIndex'],
      ],
  edges: [
        ['start', 'type'],
        ['type',  'pos'],
        ['pos',   'ports'],
        ['ports', 'props'],
        ['props', 'pids'],
      ]
};

flowRegistry['ge_load'] = {
  title: 'Load Element',
  nodes: [
        ['start',  'Load element\nfrom stream',                'start', ''],
        ['pos',    'Read position,\nrotation, flip',            'step',   ''],
        ['sizes',  'Read input/output sizes',                   'step',   ''],
        ['resize', 'Resize port arrays', 'key', 'Creates/removes ports to match saved counts', 'ge_ports'],
        ['props',  'Read element-specific\nproperties',         'step',   ''],
        ['reg',    'Register all ports\nfor reconnection',      'end',   'Maps serial ID \u2192 port for wire restoration'],
      ],
  edges: [
        ['start',  'pos'],
        ['pos',    'sizes'],
        ['sizes',  'resize'],
        ['resize', 'props'],
        ['props',  'reg'],
      ]
};

flowRegistry['ge_logic'] = {
  title: 'Update Logic',
  nodes: [
        ['start',  'Called by simulation\nin topological order', 'start', ''],
        ['snap',   'Snapshot inputs\nfrom predecessors',        'key',     'Copy predecessor outputs into input value array'],
        ['compute','Element-specific logic\n(AND, FF, etc.)',   'key',    'Polymorphic: each subclass overrides'],
        ['output', 'Set output values\nand change flag',        'end',     ''],
      ],
  edges: [
        ['start',   'snap'],
        ['snap',    'compute'],
        ['compute', 'output'],
      ]
};

flowRegistry['ge_ports'] = {
  title: 'Port Management',
  nodes: [
        ['start',   'Set input or\noutput size',                'start', ''],
        ['d_range', 'Size within\nmin..max?',                   'decision',''],
        ['r_range', 'Abort\n(out of range)',                    'error',   ''],
        ['d_grow',  'Growing or\nshrinking?',                   'decision',''],
        ['add',     'Add ports\nin loop',                       'step', ''],
        ['shrink',  'Remove excess ports',                      'step', ''],
        ['upd',     'Redistribute ports\nalong element edge',   'end',    ''],
      ],
  edges: [
        ['start',   'd_range'],
        ['d_range', 'r_range', 'No'],
        ['d_range', 'd_grow',  'Yes'],
        ['d_grow',  'add',     'Growing'],
        ['d_grow',  'shrink',  'Shrinking'],
        ['add',     'upd'],
        ['shrink',  'upd'],
      ]
};

flowRegistry['ef_build'] = {
  title: 'Build Element',
  nodes: [
        ['start',   'Build element\nby type',                  'start',    ''],
        ['d_unk',   'Type is\nunknown?',                        'decision', ''],
        ['ex_unk',  'Error:\n"Unknown element type"',          'error',''],
        ['find',    'Look up type\nin creator registry',        'step',     ''],
        ['d_found', 'Found in\nregistry?',                     'decision', ''],
        ['ex_nf',   'Error:\n"Unknown type"',                  'error',    ''],
        ['create',  'Invoke creator\n\u2192 new element',      'end',      ''],
      ],
  edges: [
        ['start',   'd_unk'],
        ['d_unk',   'ex_unk',  'Yes'],
        ['d_unk',   'find',    'No'],
        ['find',    'd_found'],
        ['d_found', 'ex_nf',   'No'],
        ['d_found', 'create',  'Yes'],
      ]
};

flowRegistry['ef_registration'] = {
  title: 'Static Registration',
  nodes: [
        ['start',   'Program startup\n(static init)',       'start',    ''],
        ['meta',    'Element metadata\nconstructor',         'step',     'For each element type (AND, OR, DFlipFlop, etc.)'],
        ['info',    'Get element info:\nconstraints, pixmap,\ntitle, name', 'step', ''],
        ['reg',     'Register creator\nfunction',            'key',     'Stores creator in registry'],
        ['ready',   'Factory ready:\n40+ types registered', 'end',     ''],
      ],
  edges: [
        ['start', 'meta'],
        ['meta',  'info'],
        ['info',  'reg'],
        ['reg',   'ready'],
      ]
};

flowRegistry['editor_apply'] = {
  title: 'Property Editing',
  nodes: [
        ['start',  'User edits property\nin right sidebar',  'start',   ''],
        ['apply',  'Apply changes',                           'key',     ''],
        ['sel',    'Get selected elements',                   'step',    ''],
        ['old',    'Snapshot current state\n(for undo)',       'step', ''],
        ['set',    'Apply property values\nfrom UI widgets',  'step',    ''],
        ['cmd', 'Push update command\nto undo stack', 'end', 'Undoable property change', 'cmd_update'],
      ],
  edges: [
        ['start', 'apply'],
        ['apply', 'sel'],
        ['sel',   'old'],
        ['old',   'set'],
        ['set',   'cmd'],
      ]
};

flowRegistry['editor_selection'] = {
  title: 'Selection Changed',
  nodes: [
        ['start',  'Scene selection changes',               'start',    ''],
        ['fn',     'Handle selection change',                'key',     ''],
        ['clear',  'Clear all UI fields',                    'step',    ''],
        ['sel',    'Get selected elements',                  'step',    ''],
        ['d_empty','Selection\nempty?',                      'decision', ''],
        ['hide',   'Hide editor panel',                      'end',     ''],
        ['merge',  'Merge capabilities\nfrom all selected',  'step',    'Union of editable properties across selection'],
        ['load',   'Load property values\ninto UI widgets',  'step',    ''],
        ['show',   'Show editor panel\nwith applicable fields','end', 'Enable/disable fields based on what the selection supports'],
      ],
  edges: [
        ['start',  'fn'],
        ['fn',     'clear'],
        ['clear',  'sel'],
        ['sel',    'd_empty'],
        ['d_empty','hide',   'Yes'],
        ['d_empty','merge',  'No'],
        ['merge',  'load'],
        ['load',   'show'],
      ]
};

flowRegistry['element_hierarchy'] = {
  title: 'Factory & Hierarchy',
  nodes: [
        ['factory',  'Element Factory:\nbuild by type',          'start',    'Lookup in creator registry'],
        ['meta',     'Element Metadata',                          'step',     'Auto-generated registrations bind type \u2192 creator. Each type registered at static init.'],
        ['base',     'GraphicElement\n(base class)',              'key',      'QGraphicsObject + ItemWithId. Manages: ports, pixmap, serialization, grid-snap, label, theme.'],
        ['gates', 'Gates', 'step', 'AND, OR, NOT, NAND, NOR, XOR, XNOR, Buffer. Logic = boolean op on inputs', 'ge_logic'],
        ['memory', 'Memory', 'step', 'DFlipFlop, JKFlipFlop, SRFlipFlop, TFlipFlop, DLatch, SRLatch. Logic = edge-triggered state machine', 'ge_logic'],
        ['inputs',   'Inputs',                                    'step',     'Button, Switch, Rotary, Clock. Push user/clock state'],
        ['outputs',  'Outputs',                                   'step',     'LED, Display, 7-Seg, Buzzer. Update visual from port status'],
        ['mux', 'Mux / Demux', 'step', 'Multiplexer, Demultiplexer. Logic = route selected input', 'ge_logic'],
        ['ic_elem', 'IC', 'key', 'Sub-circuit wrapper. Logic = run internal sim graph', 'ge_logic'],
      ],
  edges: [
        ['factory', 'meta',    'lookup'],
        ['meta',    'base',    'creates'],
        ['base',    'gates',   'subtype'],
        ['base',    'memory',  'subtype'],
        ['base',    'inputs',  'subtype'],
        ['base',    'outputs', 'subtype'],
        ['base',    'mux',     'subtype'],
        ['base',    'ic_elem', 'subtype'],
      ]
};


flowRegistry['components_registry'] = {
  title: 'Component Registry',
  nodes: [
        ['meta',     'Element Metadata\n(auto-generated)',        'start',    'Registers into factory at static init time'],
        ['and',      'AND / NAND',                               'step',     'Bitwise AND / negate'],
        ['or',       'OR / NOR',                                 'step',     'Bitwise OR / negate'],
        ['xor',      'XOR / XNOR',                              'step',     'Parity check / negate'],
        ['not',      'NOT / Buffer',                             'step',     'Invert / pass-through'],
        ['dff',      'DFlipFlop',                                'step',     'Data on rising edge'],
        ['jkff',     'JKFlipFlop',                               'step',     'J-K on rising edge'],
        ['srff',     'SRFlipFlop',                               'step',     'Set-Reset on edge'],
        ['tff',      'TFlipFlop',                                'step',     'Toggle on rising edge'],
        ['dlatch',   'DLatch',                                   'step',     'Level-sensitive D'],
        ['srlatch',  'SRLatch',                                  'step',     'Level-sensitive SR'],
        ['button',   'Button',                                   'step',     'Momentary press'],
        ['switch',   'Switch',                                   'step',     'Toggle on/off'],
        ['rotary',   'Rotary',                                   'step',     'Multi-position selector'],
        ['clock',    'Clock',                                    'step',     'Event-driven oscillator'],
        ['led',      'LED',                                      'step',     'On/Off indicator (colored)'],
        ['seg7',     '7-Segment Display',                        'step',     'Numeric digit'],
        ['buzzer',   'Buzzer',                                   'step',     'Audio output'],
        ['mux',      'Multiplexer / Demux',                     'step',     'Route by select line'],
        ['vcc',      'VCC / GND',                                'step',     'Constant 1 / 0'],
        ['other',    'Node / Line /\nText / TruthTable',        'step',     ''],
      ],
  edges: [
        ['meta', 'and',    'Gate'],
        ['meta', 'or',     'Gate'],
        ['meta', 'xor',    'Gate'],
        ['meta', 'not',    'Gate'],
        ['meta', 'dff',    'Memory'],
        ['meta', 'jkff',   'Memory'],
        ['meta', 'srff',   'Memory'],
        ['meta', 'tff',    'Memory'],
        ['meta', 'dlatch', 'Memory'],
        ['meta', 'srlatch','Memory'],
        ['meta', 'button', 'Input'],
        ['meta', 'switch', 'Input'],
        ['meta', 'rotary', 'Input'],
        ['meta', 'clock',  'Input'],
        ['meta', 'led',    'Output'],
        ['meta', 'seg7',   'Output'],
        ['meta', 'buzzer', 'Output'],
        ['meta', 'mux',    'Mux'],
        ['meta', 'vcc',    'Static'],
        ['meta', 'other',  'Other'],
      ]
};
