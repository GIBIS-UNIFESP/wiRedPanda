// Flow definitions: commands
flowRegistry['cmd_ops'] = {
  title: 'Commands \u2014 Undo/Redo System',
  nodes: [
    ['root',  'QUndoStack',                          'start', '10 QUndoCommand subclasses'],
    ['add',   'AddItemsCommand',                     'key',   'redo: loadItems, undo: saveItems + deleteItems', 'cmd_add'],
    ['del',   'DeleteItemsCommand',                  'key',   'redo: deleteItems, undo: loadItems', 'cmd_delete'],
    ['move',  'MoveCommand',                         'key',   'Swap old/new positions', 'cmd_move'],
    ['update','UpdateCommand',                       'key',   'Swap old/new serialized state', 'cmd_update'],
    ['rotate','RotateCommand',                       'step',  'Centroid rotation with 2D transform'],
    ['flip',  'FlipCommand',                         'step',  'Flip is involution: undo = redo'],
    ['morph', 'MorphCommand',                        'step',  'Change element type in-place'],
    ['split', 'SplitCommand',                        'step',  'Insert Node into wire'],
    ['blob', 'UpdateBlobCommand', 'step', 'IC embed/extract with rollback', 'cmd_update'],
    ['regblob','RegisterBlobCommand',                'step',  'Add/remove blob in ICRegistry'],
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
  title: 'AddItemsCommand',
  nodes: [
        ['ctor',    'Constructor(items, scene)',            'start',    ''],
        ['block',   'SimulationBlocker',                    'step',     ''],
        ['add',     'CommandUtils::addItems\n(scene, items)','step',   ''],
        ['save',    'loadList \u2192 save IDs\n+ other IDs','step',    ''],
        ['undo',    'undo()',                                'key',     ''],
        ['u_block', 'SimulationBlocker',                    'step',     ''],
        ['u_find',  'findItems(scene, m_ids)',              'step',     ''],
        ['u_save',  'saveItems \u2192 m_itemData',          'step',    'Serialize current state for redo'],
        ['u_del',   'deleteItems(scene, items)',            'step',     ''],
        ['u_stale', 'setCircuitUpdateRequired()',           'end',      ''],
        ['redo',    'redo()',                                'key',     ''],
        ['r_block', 'SimulationBlocker',                    'step',     ''],
        ['r_load',  'loadItems(scene,\nm_itemData, ids)',   'step',    'Deserialize from saved bytes'],
        ['r_stale', 'setCircuitUpdateRequired()',           'end',      ''],
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
  title: 'DeleteItemsCommand',
  nodes: [
        ['ctor',    'Constructor(items, scene)',            'start',    'Mirror of AddItems: redo deletes, undo restores'],
        ['redo',    'redo()',                                'key',     ''],
        ['r_block', 'SimulationBlocker',                    'step',     ''],
        ['r_find',  'findItems(scene, m_ids)',              'step',     ''],
        ['r_save',  'saveItems \u2192 m_itemData',          'step',    ''],
        ['r_del',   'deleteItems(scene, items)',            'step',     ''],
        ['r_stale', 'setCircuitUpdateRequired()',           'end',      ''],
        ['undo',    'undo()',                                'key',     ''],
        ['u_block', 'SimulationBlocker',                    'step',     ''],
        ['u_load',  'loadItems(scene,\nm_itemData, ids)',   'step',    ''],
        ['u_stale', 'setCircuitUpdateRequired()',           'end',      ''],
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
  title: 'MoveCommand',
  nodes: [
        ['ctor',    'Constructor\n(elements, oldPositions)', 'start',  'Captures old + new positions + element IDs'],
        ['undo',    'undo()',                                'key',     ''],
        ['u_find',  'findElements(scene, ids)',             'step',     ''],
        ['u_set',   'setPos(m_oldPositions[i])\nfor each element', 'step', ''],
        ['u_auto',  'setAutosaveRequired()',                'end',      ''],
        ['redo',    'redo()',                                'key',     ''],
        ['r_find',  'findElements(scene, ids)',             'step',     ''],
        ['r_set',   'setPos(m_newPositions[i])\nfor each element', 'step', ''],
        ['r_auto',  'setAutosaveRequired()',                'end',      ''],
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
  title: 'UpdateCommand',
  nodes: [
        ['ctor',    'Constructor\n(elements, oldData)',     'start',    'Saves old + new serialized state'],
        ['c_save', 'writePandaHeader +\nelement.save()\nfor each \u2192 m_newData', 'step', '', 'ser_header'],
        ['undo',    'undo()',                                'key',     ''],
        ['u_load',  'loadData(m_oldData)',                  'step',     ''],
        ['u_find', 'findElements \u2192\nreadPandaHeader \u2192\nelement.load() for each', 'step', '', 'ser_header'],
        ['u_stale', 'setCircuitUpdateRequired()',           'end',      ''],
        ['redo',    'redo()',                                'key',     ''],
        ['r_load',  'loadData(m_newData)',                  'step',     ''],
        ['r_stale', 'setCircuitUpdateRequired()',           'end',      ''],
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

