// Flow definitions: elements
flowRegistry['ge_ops'] = {
  title: 'GraphicElement \u2014 Element System',
  nodes: [
    ['root',    'Element System',                    'start', ''],
    ['factory', 'ElementFactory\nbuildElement(type)', 'key', 'Lookup creator lambda in m_creatorMap', 'ef_build'],
    ['base',    'GraphicElement\n(Base Class)',       'key', 'QGraphicsObject + ItemWithId: ports, pixmap, label, theme'],
    ['save',    'save(QDataStream)',                   'key', 'Type \u2192 pos \u2192 rotation \u2192 ports \u2192 props', 'ge_save'],
    ['load',    'load(QDataStream, ctx)',              'key', 'Version-aware, registers ports in portMap', 'ge_load'],
    ['logic',   'updateLogic()',                       'key', 'Simulation cycle: snap inputs \u2192 compute \u2192 set outputs', 'ge_logic'],
    ['ports',   'Port Management\n(setInputSize/OutputSize)', 'key', 'Add/remove ports, redistribute vertically', 'ge_ports'],
    ['editor',  'ElementEditor\n(Property Panel)',    'key', 'Edit props \u2192 UpdateCommand', 'editor_apply'],
    ['reg',     'Static Registration\n(ElementMetadata)', 'key', 'Auto-registers 40+ types at startup', 'ef_registration'],
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
  title: 'save()',
  nodes: [
        ['start',  'GraphicElement::save\n(QDataStream)',   'start',   ''],
        ['type',   'Write element type',                    'step',    ''],
        ['pos',    'Write position +\nrotation + flip',     'step',    ''],
        ['ports',  'Write input/output\nport counts',       'step',    ''],
        ['props',  'Write element-specific\nproperties',    'key',     'Label, color, frequency, delay, trigger, appearances'],
        ['pids',   'Write port serial IDs\nfor reconnection','end',    '(elementId << 16) | portIndex'],
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
  title: 'load()',
  nodes: [
        ['start',  'GraphicElement::load\n(stream, context)', 'start', ''],
        ['pos',    'Read position +\nrotation + flip',       'step',   ''],
        ['sizes',  'Read input/output sizes',                'step',   ''],
        ['resize', 'setInputSize + setOutputSize',           'key',    'Creates/removes ports to match saved counts'],
        ['props',  'Read element-specific\nproperties',      'step',   ''],
        ['reg',    'Register all ports in\ncontext.portMap',  'end',   'Serial ID \u2192 QNEPort* for connection reconnection'],
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
  title: 'updateLogic()',
  nodes: [
        ['start',  'Called by Simulation\nin topological order', 'start', ''],
        ['snap',   'simUpdateInputs()',                     'key',     'Copy predecessor outputs into m_simInputValues[]'],
        ['compute','Element-specific logic\n(AND, FF, etc.)','key',    'Polymorphic: each subclass overrides'],
        ['output', 'setOutputValue(result)',                 'end',     'Write to m_simOutputValues[] + set change flag'],
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
        ['start',   'setInputSize(size) or\nsetOutputSize(size)', 'start', ''],
        ['d_range', 'size within\nmin..max?',               'decision',''],
        ['r_range', 'return\n(out of range)',               'error',   ''],
        ['d_grow',  'size > current?',                      'decision',''],
        ['add',     'addInputPort() or\naddOutputPort()\nin loop', 'step', ''],
        ['shrink',  'qDeleteAll excess ports\nresize vector', 'step', ''],
        ['upd',     'updatePortsProperties()',               'end',    'Redistribute ports vertically along element edge'],
      ],
  edges: [
        ['start',   'd_range'],
        ['d_range', 'r_range', 'No'],
        ['d_range', 'd_grow',  'Yes'],
        ['d_grow',  'add',     'Yes'],
        ['d_grow',  'shrink',  'No'],
        ['add',     'upd'],
        ['shrink',  'upd'],
      ]
};

flowRegistry['ef_ops'] = {
  title: 'ElementFactory \u2014 Element Creation',
  nodes: [
    ['f0', 'buildElement()', 'key', '', 'ef_build'],
    ['f1', 'Static Registration', 'key', '', 'ef_registration']
  ],
  edges: [
    ['f0', 'f1']
  ]
};

flowRegistry['ef_build'] = {
  title: 'buildElement()',
  nodes: [
        ['start',   'buildElement(type)',                   'start',    ''],
        ['d_unk',   'type ==\nUnknown?',                    'decision', ''],
        ['ex_unk',  'throw PANDACEPTION\n"Unknown element type"','error',''],
        ['find',    'it = m_creatorMap.find(type)',          'step',     ''],
        ['d_found', 'Found in\ncreatorMap?',                'decision', ''],
        ['ex_nf',   'throw PANDACEPTION\n"Unknown type"',   'error',    ''],
        ['create',  'return it.value()()',                   'end',      'Invoke creator lambda \u2192 new GraphicElement subclass'],
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
        ['meta',    'ElementMetadata<T>\nconstructor',      'step',     'For each element type (AND, OR, DFlipFlop, etc.)'],
        ['info',    'ElementInfo<T>::metadata()',           'step',     'Returns constraints + pixmap + title + name'],
        ['reg',     'registerCreator\n(type, lambda)',       'key',     'Stores lambda that creates new T() in m_creatorMap'],
        ['ready',   'Factory ready:\nm_creatorMap populated','end',     '40+ element types registered'],
      ],
  edges: [
        ['start', 'meta'],
        ['meta',  'info'],
        ['info',  'reg'],
        ['reg',   'ready'],
      ]
};

flowRegistry['editor_ops'] = {
  title: 'ElementEditor \u2014 Property Panel',
  nodes: [
    ['f0', 'Property Editing', 'key', '', 'editor_apply'],
    ['f1', 'Selection Changed', 'key', '', 'editor_selection']
  ],
  edges: [
    ['f0', 'f1']
  ]
};

flowRegistry['editor_apply'] = {
  title: 'Property Editing',
  nodes: [
        ['start',  'User edits property\nin right sidebar',  'start',   ''],
        ['apply',  'apply()',                                 'key',     ''],
        ['sel',    'Get selected elements',                  'step',    ''],
        ['old',    'Snapshot oldData\n(serialize current state)','step', ''],
        ['set',    'Apply property values\nfrom UI widgets', 'step',    ''],
        ['cmd',    'UpdateCommand\n(elements, oldData)\n\u2192 undoStack', 'end', 'Undoable property change'],
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
        ['fn',     'selectionChanged()',                     'key',     ''],
        ['clear',  'Clear all UI fields',                   'step',    ''],
        ['sel',    'Get selected elements',                 'step',    ''],
        ['d_empty','Selection\nempty?',                     'decision', ''],
        ['hide',   'Hide editor panel',                     'end',     ''],
        ['merge',  'Merge capabilities\nfrom all selected', 'step',    'Union of editable properties across selection'],
        ['load',   'Load property values\ninto UI widgets', 'step',    ''],
        ['show',   'Show editor panel\napplyCapabilitiesToUi()','end', 'Enable/disable fields based on what the selection supports'],
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

flowRegistry['element_mod'] = {
  title: 'Element System',
  nodes: [
    ['f0', 'Factory & Hierarchy', 'key', '', 'element_hierarchy'],
    ['f1', 'Simulation Interface', 'key', '', 'element_sim_iface'],
    ['f2', 'Serialization', 'key', '', 'element_serial']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2']
  ]
};

flowRegistry['element_hierarchy'] = {
  title: 'Factory & Hierarchy',
  nodes: [
        ['factory',  'ElementFactory::\nbuildElement(type)',     'start',    'Lookup in m_creatorMap'],
        ['meta',     'ElementMetadata',                         'step',     'Auto-generated registrations bind ElementType \u2192 creator lambda. Each type registered at static init.'],
        ['base',     'GraphicElement\n(base class)',            'key',      'QGraphicsObject + ItemWithId. Manages: ports, pixmap, serialization, grid-snap, label, theme.'],
        ['gates',    'Gates',                                   'step',     'AND, OR, NOT, NAND, NOR, XOR, XNOR, Buffer. updateLogic() = boolean op on inputs'],
        ['memory',   'Memory',                                  'step',     'DFlipFlop, JKFlipFlop, SRFlipFlop, TFlipFlop, DLatch, SRLatch. updateLogic() = edge-triggered state machine'],
        ['inputs',   'Inputs',                                  'step',     'Button, Switch, Rotary, Clock. updateOutputs() = push user/clock state'],
        ['outputs',  'Outputs',                                 'step',     'LED, Display, 7-Seg, Buzzer. refresh() = update visual from port status'],
        ['mux',      'Mux / Demux',                             'step',     'Multiplexer, Demultiplexer. updateLogic() = route selected input'],
        ['ic_elem',  'IC',                                      'key',      'Sub-circuit wrapper. updateLogic() = run internal sim graph'],
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

flowRegistry['element_sim_iface'] = {
  title: 'Simulation Interface',
  nodes: [
        ['data',     'Direct Simulation Data',                   'start',    'm_simInputConnections[], m_simInputValues[], m_simOutputValues[], connectPredecessor()'],
        ['snap',     'simUpdateInputs()',                        'step',     'Snapshot predecessors\u2019 outputs into input cache'],
        ['compute',  'Compute element-specific\nlogic (AND, FF, ...)', 'key', ''],
        ['output',   'setOutputValue()',                         'end',      'Write results + set change flag'],
      ],
  edges: [
        ['data',    'snap'],
        ['snap',    'compute'],
        ['compute', 'output'],
      ]
};

flowRegistry['element_serial'] = {
  title: 'Serialization',
  nodes: [
        ['save',     'save(QDataStream)',                        'start',    'Type \u2192 position \u2192 rotation \u2192 ports \u2192 label \u2192 trigger \u2192 appearances'],
        ['load',     'load(QDataStream, ctx)',                   'key',      'Version-aware: V4.6+ new format vs legacy. Rebuilds ports from stream, clamps to constraints.'],
      ],
  edges: []
};

flowRegistry['components_registry'] = {
  title: 'Component Registry',
  nodes: [
        ['meta',     'ElementMetadata\n(auto-generated)',        'start',    'Registers into ElementFactory at static init time'],
        ['and',      'AND / NAND',                               'step',     'std::bit_and / negate'],
        ['or',       'OR / NOR',                                 'step',     'std::bit_or / negate'],
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

