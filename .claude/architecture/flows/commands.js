// Flow definitions: commands
flowRegistry['commands_ops'] = {
  title: 'Commands \u2014 Undo/Redo System',
  nodes: [
    ['f0', '\u2460 AddItemsCommand', 'key', '', 'commands_u2460_additemscommand'],
    ['f1', '\u2461 DeleteItemsCommand', 'key', '', 'commands_u2461_deleteitemscommand'],
    ['f2', '\u2462 MoveCommand', 'key', '', 'commands_u2462_movecommand'],
    ['f3', '\u2463 UpdateCommand', 'key', '', 'commands_u2463_updatecommand']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2'],
    ['f2', 'f3']
  ]
};

flowRegistry['commands_u2460_additemscommand'] = {
  title: '\u2460 AddItemsCommand',
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

flowRegistry['commands_u2461_deleteitemscommand'] = {
  title: '\u2461 DeleteItemsCommand',
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

flowRegistry['commands_u2462_movecommand'] = {
  title: '\u2462 MoveCommand',
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

flowRegistry['commands_u2463_updatecommand'] = {
  title: '\u2463 UpdateCommand',
  nodes: [
        ['ctor',    'Constructor\n(elements, oldData)',     'start',    'Saves old + new serialized state'],
        ['c_save',  'writePandaHeader +\nelement.save()\nfor each \u2192 m_newData', 'step', ''],
        ['undo',    'undo()',                                'key',     ''],
        ['u_load',  'loadData(m_oldData)',                  'step',     ''],
        ['u_find',  'findElements \u2192\nreadPandaHeader \u2192\nelement.load() for each', 'step', ''],
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

