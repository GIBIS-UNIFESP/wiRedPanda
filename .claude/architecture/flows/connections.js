// Flow definitions: connections
flowRegistry['conn_mgr_ops'] = {
  title: 'ConnectionManager \u2014 Wire Operations',
  nodes: [
    ['f0', '\u2460 Wire Creation', 'key', '', 'conn_mgr_u2460_wire_creation'],
    ['f1', '\u2461 Connection Validation', 'key', '', 'conn_mgr_u2461_connection_validation']
  ],
  edges: [
    ['f0', 'f1']
  ]
};

flowRegistry['conn_mgr_u2460_wire_creation'] = {
  title: '\u2460 Wire Creation',
  nodes: [
        ['start_o', 'startFromOutput(port)',               'start',    'User clicks an output port'],
        ['new_c',   'new QNEConnection()',                  'step',    ''],
        ['set_sp',  'setStartPort(port)\nsetEndPos(mousePos)','step', ''],
        ['add',     'scene->addItem(connection)',            'step',    ''],
        ['set_ed',  'setEditedConnection(conn)',            'step',    ''],
        ['upd_p',   'updatePath()',                          'step',    ''],
        ['move',    'User drags mouse\nupdateEditedEnd(pos)', 'key',  'Called from Scene::mouseMoveEvent'],
        ['release', 'User releases\ntryComplete(scenePos)',  'key',   ''],
        ['find_p',  'Find QNEPort at pos',                  'step',    ''],
        ['d_port',  'Port found?',                          'decision',''],
        ['r_nop',   'deleteEditedConnection()',             'error',   'No port under cursor'],
        ['resolve', 'Resolve start/end\nfrom connection + port','step','If conn has start \u2192 port is end, and vice versa'],
        ['d_valid', 'isConnectionAllowed\n(start, end)?',   'decision',''],
        ['reject',  'deleteEditedConnection()',             'error',   'Connection not allowed'],
        ['wire',    'setStartPort + setEndPort',            'step',    ''],
        ['cmd',     'AddItemsCommand\n\u2192 undoStack',    'end',    ''],
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

flowRegistry['conn_mgr_u2461_connection_validation'] = {
  title: '\u2461 Connection Validation',
  nodes: [
        ['start',   'isConnectionAllowed\n(startPort, endPort)', 'start', ''],
        ['d_null',  'Either port\nnull?',                   'decision',''],
        ['r_null',  'return false',                          'error',  ''],
        ['d_self',  'Same parent\nelement?',                'decision','Self-loop check'],
        ['r_self',  'return false',                          'error',  ''],
        ['d_dup',   'Already\nconnected?',                  'decision','Duplicate wire check'],
        ['r_dup',   'return false',                          'error',  ''],
        ['d_rx',    'End element\nis wireless Rx?',         'decision','Rx nodes receive over air, not wire'],
        ['r_rx',    'return false',                          'error',  ''],
        ['d_tx',    'Start element\nis wireless Tx?',       'decision','Tx nodes broadcast only'],
        ['r_tx',    'return false',                          'error',  ''],
        ['ok',      'return true',                           'end',    ''],
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

flowRegistry['nodes_mod'] = {
  title: 'Nodes (Wiring)',
  nodes: [
    ['f0', 'Port & Connection System', 'key', '', 'nodes_port__connection_system'],
    ['f1', 'Wire Creation', 'key', '', 'nodes_wire_creation'],
    ['f2', 'Simulation Role', 'key', '', 'nodes_simulation_role']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2']
  ]
};

flowRegistry['nodes_port__connection_system'] = {
  title: 'Port & Connection System',
  nodes: [
        ['port',     'QNEPort (base)',                           'start',    'QGraphicsItem on element edge. Holds: connections list, status, index, default value. Visual: colored circle.'],
        ['inp',      'QNEInputPort',                             'step',     'Left side of element. Accepts one incoming connection. Has defaultValue() for unconnected state.'],
        ['outp',     'QNEOutputPort',                            'step',     'Right side of element. Can have multiple outgoing connections. Status set by setOutputValue().'],
        ['conn',     'QNEConnection',                            'key',      'QGraphicsItem: bezier curve wire. startPort() \u2192 output, endPort() \u2192 input. Updates path on port move.'],
      ],
  edges: [
        ['port', 'inp',  'subtype'],
        ['port', 'outp', 'subtype'],
        ['inp',  'conn', 'linked by'],
        ['outp', 'conn', 'linked by'],
      ]
};

flowRegistry['nodes_wire_creation'] = {
  title: 'Wire Creation',
  nodes: [
        ['drag',     'User drags from\noutput port',             'start',    'ConnectionManager creates temporary QNEConnection'],
        ['hover',    'Hover over input port',                    'step',     'isConnectionAllowed() validates: not self-loop, compatible types, not already connected'],
        ['d_ok',     'Connection\nallowed?',                     'decision', ''],
        ['reject',   'Reject\n(visual feedback)',                'error',    ''],
        ['release',  'Release on input port',                    'step',     ''],
        ['cmd',      'AddItemsCommand\n\u2192 undo stack',      'key',      'Push to undo stack \u2192 mark sim stale'],
      ],
  edges: [
        ['drag',    'hover'],
        ['hover',   'd_ok'],
        ['d_ok',    'reject',  'No'],
        ['d_ok',    'release', 'Yes'],
        ['release', 'cmd'],
      ]
};

flowRegistry['nodes_simulation_role'] = {
  title: 'Simulation Role',
  nodes: [
        ['note',     'Connections propagate,\nthey don\u2019t compute', 'key', ''],
        ['up_out',   'updatePort(outputPort)',                   'step',     'Copy element output \u2192 port status'],
        ['up_inp',   'updatePort(inputPort)',                    'step',     'Copy connected output port status \u2192 input port'],
        ['bypass',   'Sim graph bypasses\nconnections entirely', 'end',     'Uses connectPredecessor() for direct element-to-element links'],
      ],
  edges: [
        ['note',   'up_out'],
        ['up_out', 'up_inp'],
        ['up_inp', 'bypass'],
      ]
};

