// Flow definitions: commands
flowRegistry['cmd_ops'] = {
  title: 'Commands \u2014 Undo/Redo System',
  nodes: [
    ['root',  'Undo Stack',                          'start', '10 command subclasses'],
    ['add',   'Add Items',                           'key',   'redo: load items, undo: save + delete items', 'cmd_add'],
    ['del',   'Delete Items',                        'key',   'redo: delete items, undo: load items', 'cmd_delete'],
    ['move',  'Move',                                'key',   'Swap old/new positions', 'cmd_move'],
    ['update','Update',                              'key',   'Swap old/new serialized state', 'cmd_update'],
    ['rotate','Rotate',                              'step',  'Centroid rotation with 2D transform'],
    ['flip',  'Flip',                                'step',  'Flip is involution: undo = redo'],
    ['morph', 'Morph',                               'step',  'Change element type in-place'],
    ['split', 'Split',                               'step',  'Insert Node into wire'],
    ['blob', 'Update Blob', 'step', 'IC embed/extract with rollback', 'cmd_update'],
    ['regblob','Register Blob',                      'step',  'Add/remove blob in IC Registry'],
  ],
  edges: [
    ['root',  'add'],
    ['root',  'del'],
    ['root',  'move'],
    ['root',  'update'],
    ['root',  'rotate'],
    ['root',  'flip'],
    ['root',  'morph'],
    ['root',  'split'],
    ['root',  'blob'],
    ['root',  'regblob'],
  ]
};

flowRegistry['cmd_add'] = {
  title: 'Add Items Command',
  nodes: [
        ['ctor',    'Construct\n(items, scene)',               'start',    ''],
        ['block',   'Pause Simulation',                         'step',     ''],
        ['add',     'Add items to scene',                       'step',    ''],
        ['save',    'Save item IDs',                            'step',    ''],
        ['undo',    'Undo',                                     'key',     ''],
        ['u_block', 'Pause Simulation',                         'step',     ''],
        ['u_find',  'Find items by ID',                         'step',     ''],
        ['u_save',  'Save current state\nfor redo',             'step',    'Serialize current state'],
        ['u_del',   'Delete items\nfrom scene',                 'step',     ''],
        ['u_stale', 'Mark circuit\nfor rebuild',                'end',      ''],
        ['redo',    'Redo',                                     'key',     ''],
        ['r_block', 'Pause Simulation',                         'step',     ''],
        ['r_load',  'Restore items\nfrom saved state',          'step',    'Deserialize from saved bytes'],
        ['r_stale', 'Mark circuit\nfor rebuild',                'end',      ''],
      ],
  edges: [
        ['ctor',    'block'],
        ['block',   'add'],
        ['add',     'save'],
        ['undo',    'u_block'],
        ['u_block', 'u_find'],
        ['u_find',  'u_save'],
        ['u_save',  'u_del'],
        ['u_del',   'u_stale'],
        ['redo',    'r_block'],
        ['r_block', 'r_load'],
        ['r_load',  'r_stale'],
      ]
};

flowRegistry['cmd_delete'] = {
  title: 'Delete Items Command',
  nodes: [
        ['ctor',    'Construct\n(items, scene)',               'start',    'Mirror of Add Items: redo deletes, undo restores'],
        ['redo',    'Redo',                                     'key',     ''],
        ['r_block', 'Pause Simulation',                         'step',     ''],
        ['r_find',  'Find items by ID',                         'step',     ''],
        ['r_save',  'Save current state',                       'step',    ''],
        ['r_del',   'Delete items\nfrom scene',                 'step',     ''],
        ['r_stale', 'Mark circuit\nfor rebuild',                'end',      ''],
        ['undo',    'Undo',                                     'key',     ''],
        ['u_block', 'Pause Simulation',                         'step',     ''],
        ['u_load',  'Restore items\nfrom saved state',          'step',    ''],
        ['u_stale', 'Mark circuit\nfor rebuild',                'end',      ''],
      ],
  edges: [
        ['ctor',    'redo'],
        ['redo',    'r_block'],
        ['r_block', 'r_find'],
        ['r_find',  'r_save'],
        ['r_save',  'r_del'],
        ['r_del',   'r_stale'],
        ['undo',    'u_block'],
        ['u_block', 'u_load'],
        ['u_load',  'u_stale'],
      ]
};

flowRegistry['cmd_move'] = {
  title: 'Move Command',
  nodes: [
        ['ctor',    'Construct\n(elements, old positions)', 'start',  'Captures old + new positions + element IDs'],
        ['undo',    'Undo',                                     'key',     ''],
        ['u_find',  'Find elements by ID',                      'step',     ''],
        ['u_set',   'Restore old positions',                    'step',    ''],
        ['u_auto',  'Mark for autosave',                        'end',      ''],
        ['redo',    'Redo',                                     'key',     ''],
        ['r_find',  'Find elements by ID',                      'step',     ''],
        ['r_set',   'Apply new positions',                      'step',    ''],
        ['r_auto',  'Mark for autosave',                        'end',      ''],
      ],
  edges: [
        ['undo',   'u_find'],
        ['u_find', 'u_set'],
        ['u_set',  'u_auto'],
        ['redo',   'r_find'],
        ['r_find', 'r_set'],
        ['r_set',  'r_auto'],
      ]
};

flowRegistry['cmd_update'] = {
  title: 'Update Command',
  nodes: [
        ['ctor',    'Construct\n(elements, old state)',        'start',    'Saves old + new serialized state'],
        ['c_save', 'Serialize current state\nas new data', 'step', '', 'ser_header'],
        ['undo',    'Undo',                                     'key',     ''],
        ['u_load',  'Load old state',                           'step',     ''],
        ['u_find', 'Find elements,\nread header,\nrestore each', 'step', '', 'ser_header'],
        ['u_stale', 'Mark circuit\nfor rebuild',                'end',      ''],
        ['redo',    'Redo',                                     'key',     ''],
        ['r_load',  'Load new state',                           'step',     ''],
        ['r_stale', 'Mark circuit\nfor rebuild',                'end',      ''],
      ],
  edges: [
        ['ctor',    'c_save'],
        ['undo',    'u_load'],
        ['u_load',  'u_find'],
        ['u_find',  'u_stale'],
        ['redo',    'r_load'],
        ['r_load',  'r_stale'],
      ]
};
