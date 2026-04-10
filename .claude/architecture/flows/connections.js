// Flow definitions: connections
flowRegistry['conn_ops'] = {
  title: 'Connections \u2014 Wiring System',
  nodes: [
    ['root',    'Wiring System',                     'start', ''],
    ['ports',   'QNEPort\n(Input / Output)',         'step',  'QGraphicsItem on element edge, colored by signal status'],
    ['conn',    'QNEConnection\n(Wire)',              'step', 'Bezier curve between ports, updates path on move'],
    ['create',  'Wire Creation\n(Connection Manager)', 'key', 'Drag from port \u2192 validate \u2192 complete', 'conn_create'],
    ['validate','Connection\nValidation',             'key', '5 checks: null, self-loop, duplicate, Rx, Tx', 'conn_validate'],
    ['sim_role','Simulation Role',                     'step', 'Connections propagate, don\u2019t compute. Sim graph links elements directly.'],
  ],
  edges: [
    ['root',     'ports'],
    ['root',     'conn'],
    ['root',     'create'],
    ['create',   'validate'],
    ['conn',     'sim_role'],
  ]
};

flowRegistry['conn_create'] = {
  title: 'Wire Creation',
  nodes: [
        ['start_o', 'User clicks\nan output port',                'start',    ''],
        ['new_c',   'Create new wire',                             'step',    ''],
        ['set_sp',  'Set start port\nand end position',           'step', ''],
        ['add',     'Add wire to scene',                           'step',    ''],
        ['set_ed',  'Set as active\nwire edit',                    'step',    ''],
        ['upd_p',   'Update wire path',                            'step',    ''],
        ['move',    'User drags mouse\n(update wire endpoint)',    'key',  ''],
        ['release', 'User releases\n(try to complete)',            'key',   ''],
        ['find_p',  'Find port at\nrelease position',              'step',    ''],
        ['d_port',  'Port found?',                                 'decision',''],
        ['r_nop',   'Delete wire\n(no port under cursor)',        'error',   ''],
        ['resolve', 'Resolve start/end\nfrom wire + port',        'step','If wire has start \u2192 port is end, and vice versa'],
        ['d_valid', 'Connection\nallowed?',                        'decision',''],
        ['reject',  'Delete wire\n(not allowed)',                  'error',   ''],
        ['wire',    'Set start and\nend ports',                    'step',    ''],
        ['cmd', 'Push add command\nto undo stack', 'end', '', 'cmd_add'],
      ],
  edges: [
        ['start_o', 'new_c'],
        ['new_c',   'set_sp'],
        ['set_sp',  'add'],
        ['add',     'set_ed'],
        ['set_ed',  'upd_p'],
        ['upd_p',   'move'],
        ['move',    'release'],
        ['release', 'find_p'],
        ['find_p',  'd_port'],
        ['d_port',  'r_nop',    'No'],
        ['d_port',  'resolve',  'Yes'],
        ['resolve', 'd_valid'],
        ['d_valid', 'reject',   'No'],
        ['d_valid', 'wire',     'Yes'],
        ['wire',    'cmd'],
      ]
};

flowRegistry['conn_validate'] = {
  title: 'Connection Validation',
  nodes: [
        ['start',   'Validate connection\n(start \u2192 end port)', 'start', ''],
        ['d_null',  'Either port\nnull?',                   'decision',''],
        ['r_null',  'Reject',                                'error',  ''],
        ['d_self',  'Same parent\nelement?',                 'decision','Self-loop check'],
        ['r_self',  'Reject',                                'error',  ''],
        ['d_dup',   'Already\nconnected?',                   'decision','Duplicate wire check'],
        ['r_dup',   'Reject',                                'error',  ''],
        ['d_rx',    'End element\nis wireless Rx?',          'decision','Rx nodes receive over air, not wire'],
        ['r_rx',    'Reject',                                'error',  ''],
        ['d_tx',    'Start element\nis wireless Tx?',        'decision','Tx nodes broadcast only'],
        ['r_tx',    'Reject',                                'error',  ''],
        ['ok',      'Allow connection',                      'end',    ''],
      ],
  edges: [
        ['start',  'd_null'],
        ['d_null', 'r_null',  'Yes'],
        ['d_null', 'd_self',  'No'],
        ['d_self', 'r_self',  'Yes'],
        ['d_self', 'd_dup',   'No'],
        ['d_dup',  'r_dup',   'Yes'],
        ['d_dup',  'd_rx',    'No'],
        ['d_rx',   'r_rx',    'Yes'],
        ['d_rx',   'd_tx',    'No'],
        ['d_tx',   'r_tx',    'Yes'],
        ['d_tx',   'ok',      'No'],
      ]
};

flowRegistry['nodes_ports'] = {
  title: 'Port & Connection System',
  nodes: [
        ['port',     'QNEPort (base)',                           'start',    'QGraphicsItem on element edge. Holds: connections list, status, index, default value.'],
        ['inp',      'Input Port',                                'step',     'Left side of element. Accepts one incoming connection. Has default value for unconnected state.'],
        ['outp',     'Output Port',                               'step',     'Right side of element. Can have multiple outgoing connections. Status set by simulation.'],
        ['conn',     'Connection (Wire)',                         'key',      'QGraphicsItem: bezier curve wire. Links output \u2192 input. Updates path on port move.'],
      ],
  edges: [
        ['port', 'inp',  'subtype'],
        ['port', 'outp', 'subtype'],
        ['inp',  'conn', 'linked by'],
        ['outp', 'conn', 'linked by'],
      ]
};

