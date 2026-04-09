// Flow definitions: scene
flowRegistry['scene_ops'] = {
  title: 'Scene \u2014 Circuit Editing Operations',
  nodes: [
    ['f0', '\u2460 Copy / Cut / Paste', 'key', '', 'scene_u2460_copy__cut__paste'],
    ['f1', '\u2461 Delete Selected', 'key', '', 'scene_u2461_delete_selected'],
    ['f2', '\u2462 Rotate / Flip', 'key', '', 'scene_u2462_rotate__flip'],
    ['f3', '\u2463 Drop Element', 'key', '', 'scene_u2463_drop_element'],
    ['f4', '\u2464 Mouse Events', 'key', '', 'scene_u2464_mouse_events']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2'],
    ['f2', 'f3'],
    ['f3', 'f4']
  ]
};

flowRegistry['scene_u2460_copy__cut__paste'] = {
  title: '\u2460 Copy / Cut / Paste',
  nodes: [
        ['copy',     'Scene::copyAction()',               'start',    ''],
        ['cm_copy',  'ClipboardManager::copy()',          'key',      ''],
        ['d_sel',    'selectedElements\nempty?',           'decision', ''],
        ['clear_cb', 'clipboard->clear()\nreturn',        'error',    ''],
        ['ser',      'writePandaHeader +\nserializeItems(selected)', 'step', 'Serialize with centroid for paste offset'],
        ['blobs',    'Collect used blobs\nfrom ICRegistry','step',    'For each embedded IC in selection'],
        ['set_mime', 'setMimeData(\nclipboard + blobregistry)', 'end', ''],
        ['cut',      'Scene::cutAction()',                 'start',    ''],
        ['cm_cut',   'ClipboardManager::cut()',            'key',      ''],
        ['d_sel2',   'selectedElements\nempty?',           'decision', ''],
        ['clear_cb2','clipboard->clear()\nreturn',        'error',    ''],
        ['ser_del',  'serializeAndDelete\n(selected, stream)','step', 'Serialize + delete in one step'],
        ['set_mime2','setMimeData',                        'end',      ''],
        ['paste',    'Scene::pasteAction()',                'start',    ''],
        ['cm_paste', 'ClipboardManager::paste()',          'key',      ''],
        ['d_mime',   'mimeData\nexists?',                  'decision', ''],
        ['r_mime',   'return',                             'error',    ''],
        ['d_blobs',  'Has blob\nregistry?',               'decision', ''],
        ['imp_blob', 'Import blobs into\nICRegistry (skip dupes)','step',''],
        ['d_fmt',    'Has clipboard\nformat?',             'decision', ''],
        ['r_fmt',    'return\n(wrong format)',             'error',    ''],
        ['deser',    'readPandaHeader +\ndeserializeAndAdd','key',    'clearSelection, compute offset from centroid to mouse pos, AddItemsCommand, reposition'],
      ],
  edges: [
        ['copy',     'cm_copy'],
        ['cm_copy',  'd_sel'],
        ['d_sel',    'clear_cb',  'Yes'],
        ['d_sel',    'ser',       'No'],
        ['ser',      'blobs'],
        ['blobs',    'set_mime'],
        ['cut',      'cm_cut'],
        ['cm_cut',   'd_sel2'],
        ['d_sel2',   'clear_cb2', 'Yes'],
        ['d_sel2',   'ser_del',   'No'],
        ['ser_del',  'set_mime2'],
        ['paste',    'cm_paste'],
        ['cm_paste', 'd_mime'],
        ['d_mime',   'r_mime',    'No'],
        ['d_mime',   'd_blobs',   'Yes'],
        ['d_blobs',  'imp_blob',  'Yes'],
        ['d_blobs',  'd_fmt',     'No'],
        ['imp_blob', 'd_fmt'],
        ['d_fmt',    'r_fmt',     'No'],
        ['d_fmt',    'deser',     'Yes'],
      ]
};

flowRegistry['scene_u2461_delete_selected'] = {
  title: '\u2461 Delete Selected',
  nodes: [
        ['start',  'Scene::deleteAction()',                'start',    ''],
        ['sel',    'selectedItems()',                       'step',     ''],
        ['d_empty','empty?',                               'decision', ''],
        ['r_emp',  'return',                               'error',    ''],
        ['clear',  'clearSelection()',                      'step',     ''],
        ['cmd',    'DeleteItemsCommand\n\u2192 undoStack', 'key',      ''],
        ['restart','simulation.restart()',                  'end',      'Rebuild sim graph without deleted elements'],
      ],
  edges: [
        ['start',  'sel'],
        ['sel',    'd_empty'],
        ['d_empty','r_emp',   'Yes'],
        ['d_empty','clear',   'No'],
        ['clear',  'cmd'],
        ['cmd',    'restart'],
      ]
};

flowRegistry['scene_u2462_rotate__flip'] = {
  title: '\u2462 Rotate / Flip',
  nodes: [
        ['rot_r',   'rotateRight()',                       'start',    'rotate(90)'],
        ['rot_l',   'rotateLeft()',                        'start',    'rotate(-90)'],
        ['rotate',  'rotate(angle)',                       'key',      ''],
        ['d_sel',   'selectedElements\nempty?',            'decision', ''],
        ['r_emp',   'return',                              'error',    ''],
        ['rot_cmd', 'RotateCommand\n(elements, angle)\n\u2192 undoStack', 'end', 'Computes centroid, applies 2D rotation transform to all positions'],
        ['flip_h',  'flipHorizontally()',                  'start',    ''],
        ['flip_v',  'flipVertically()',                    'start',    ''],
        ['d_sel2',  'selectedElements\nempty?',            'decision', ''],
        ['r_emp2',  'return',                              'error',    ''],
        ['flip_cmd','FlipCommand\n(elements, axis)\n\u2192 undoStack', 'end', 'Flip is involution: undo() calls redo()'],
      ],
  edges: [
        ['rot_r',   'rotate'],
        ['rot_l',   'rotate'],
        ['rotate',  'd_sel'],
        ['d_sel',   'r_emp',    'Yes'],
        ['d_sel',   'rot_cmd',  'No'],
        ['flip_h',  'd_sel2'],
        ['flip_v',  'd_sel2'],
        ['d_sel2',  'r_emp2',   'Yes'],
        ['d_sel2',  'flip_cmd', 'No'],
      ]
};

flowRegistry['scene_u2463_drop_element'] = {
  title: '\u2463 Drop Element',
  nodes: [
        ['start',   'Scene::dropEvent(event)',             'start',    'Element dragged from palette or Ctrl+drag clone'],
        ['d_fmt',   'Mime format?',                        'decision', ''],
        ['new_elm',  'New element from palette',           'step',     'Read: offset, type, icFileName from stream'],
        ['clone',    'Clone drag (Ctrl+drag)',             'step',     'Read: offset, centroid from stream'],
        ['build',    'ElementFactory::\nbuildElement(type)','key',     ''],
        ['drop_load','loadFromDrop\n(icFileName, contextDir)','step', ''],
        ['add_cmd',  'AddItemsCommand\n\u2192 undoStack',  'key',     ''],
        ['pos',      'setPos(event\u2192scenePos - offset)','step',    ''],
        ['select',   'clearSelection()\nelement.setSelected(true)','end',''],
        ['deser_c',  'Deserialize items\nfrom clone stream','key',     ''],
        ['add_cmd2', 'AddItemsCommand\n\u2192 undoStack',  'step',    ''],
        ['repos',    'Reposition items\nby offset from mouse','step', ''],
        ['resize',   'resizeScene()',                       'end',     ''],
      ],
  edges: [
        ['start',    'd_fmt'],
        ['d_fmt',    'new_elm',   'palette drag'],
        ['d_fmt',    'clone',     'Ctrl+drag'],
        ['new_elm',  'build'],
        ['build',    'drop_load'],
        ['drop_load','add_cmd'],
        ['add_cmd',  'pos'],
        ['pos',      'select'],
        ['clone',    'deser_c'],
        ['deser_c',  'add_cmd2'],
        ['add_cmd2', 'repos'],
        ['repos',    'resize'],
      ]
};

flowRegistry['scene_u2464_mouse_events'] = {
  title: '\u2464 Mouse Events',
  nodes: [
        ['press',   'mousePressEvent()',                   'start',    ''],
        ['hover',   'connectionManager\n.updateHover(pos)', 'step',   ''],
        ['d_item',  'Item at\nclick pos?',                 'decision', ''],
        ['d_port',  'Item is\nQNEPort?',                   'decision', ''],
        ['d_editing','Has edited\nconnection?',            'decision', ''],
        ['try_comp','connectionManager\n.tryComplete(pos)', 'end',    'Complete or cancel the wire'],
        ['d_out',   'Port is\noutput?',                    'decision', ''],
        ['start_w', 'startFromOutput(port)',               'end',     'Begin new wire from output port'],
        ['d_inp_c', 'Input has\nconnections?',             'decision', ''],
        ['start_i', 'startFromInput(port)',                'end',     'Begin new wire from empty input'],
        ['detach',  'detach(port)',                         'end',     'Disconnect existing wire and drag it'],
        ['d_btn',   'Left button?',                        'decision', ''],
        ['sel_box', 'startSelectionRect()',                 'end',     'Begin rubber-band selection'],
        ['ctx',     'contextMenu(screenPos)',               'end',     'Right-click menu'],
        ['drag',    'Snapshot positions\nm_draggingElement=true', 'step', ''],
        ['move',    'mouseMoveEvent()',                     'key',     ''],
        ['d_drag',  'dragging\nelement?',                  'decision', ''],
        ['resize_s','resizeScene()\nensureVisible()',       'step',    ''],
        ['d_edit',  'Has edited\nconnection?',             'decision', ''],
        ['upd_end', 'updateEditedEnd(pos)',                 'step',    'Rubber-band the wire endpoint to mouse'],
        ['d_selbox','Selection\nbox active?',              'decision', ''],
        ['upd_rect','Update selection rect\nsetSelectionArea()','step',''],
        ['release', 'mouseReleaseEvent()',                  'key',     ''],
        ['d_moved', 'Elements\nmoved?',                    'decision', ''],
        ['mov_cmd', 'MoveCommand\n\u2192 undoStack',       'end',     ''],
        ['no_move', 'No-op\n(click without drag)',         'step',    ''],
        ['d_wire',  'Has edited\nconnection?',             'decision', ''],
        ['try_c2',  'tryComplete(pos)',                     'end',     ''],
      ],
  edges: [
        ['press',    'hover'],
        ['hover',    'd_item'],
        ['d_item',   'd_btn',     'No (empty)'],
        ['d_item',   'd_port',    'Yes'],
        ['d_port',   'd_editing', 'Yes (port)'],
        ['d_port',   'drag',      'No (element)'],
        ['d_editing','try_comp',  'Yes'],
        ['d_editing','d_out',     'No'],
        ['d_out',    'start_w',   'Yes'],
        ['d_out',    'd_inp_c',   'No (input)'],
        ['d_inp_c',  'start_i',   'No'],
        ['d_inp_c',  'detach',    'Yes'],
        ['d_btn',    'sel_box',   'Yes (left)'],
        ['d_btn',    'ctx',       'No (right)'],
        ['drag',     'move'],
        ['move',     'd_drag'],
        ['d_drag',   'resize_s',  'Yes'],
        ['d_drag',   'd_edit',    'No'],
        ['d_edit',   'upd_end',   'Yes'],
        ['d_edit',   'd_selbox',  'No'],
        ['d_selbox', 'upd_rect',  'Yes'],
        ['resize_s', 'release'],
        ['upd_end',  'release'],
        ['upd_rect', 'release'],
        ['release',  'd_moved'],
        ['d_moved',  'mov_cmd',   'Yes'],
        ['d_moved',  'no_move',   'No'],
        ['no_move',  'd_wire'],
        ['d_wire',   'try_c2',    'Yes'],
      ]
};

flowRegistry['scene_mod'] = {
  title: 'Scene',
  nodes: [
    ['f0', 'Ownership & Managers', 'key', '', 'scene_ownership__managers'],
    ['f1', 'Mouse Interactions', 'key', '', 'scene_mouse_interactions']
  ],
  edges: [
    ['f0', 'f1']
  ]
};

flowRegistry['scene_ownership__managers'] = {
  title: 'Ownership & Managers',
  nodes: [
        ['scene',    'Scene constructed',       'start',    ''],
        ['undo',     'QUndoStack',              'step',     'Undo/Redo via QUndoCommand subclasses'],
        ['sim',      'Simulation',              'key',      '1ms timer, topological sim'],
        ['icreg',    'ICRegistry',              'step',     'Caches IC definitions, file watching'],
        ['elemreg',  'Element Registry',        'step',     'Map<int, ItemWithId*> for ID lookup'],
        ['connmgr',  'ConnectionManager',       'step',     'Wire creation, validation, hover feedback'],
        ['clipmgr',  'ClipboardManager',        'step',     'Copy/Cut/Paste, clone-drag serialization'],
        ['vismgr',   'VisibilityManager',       'step',     'Show/Hide gates and wires'],
        ['propmgr',  'PropertyShortcutHandler', 'step',     'Keyboard shortcuts for cycling element properties'],
      ],
  edges: [
        ['scene', 'undo',    'owns'],
        ['scene', 'sim',     'owns'],
        ['scene', 'icreg',   'owns'],
        ['scene', 'elemreg', 'owns'],
        ['scene', 'connmgr', 'delegates'],
        ['scene', 'clipmgr', 'delegates'],
        ['scene', 'vismgr',  'delegates'],
        ['scene', 'propmgr', 'delegates'],
      ]
};

flowRegistry['scene_mouse_interactions'] = {
  title: 'Mouse Interactions',
  nodes: [
        ['press',    'mousePressEvent',         'start',    'Start selection box, start dragging element, start wire'],
        ['move',     'mouseMoveEvent',          'step',     'Update drag position, extend wire/selection'],
        ['release',  'mouseReleaseEvent',       'step',     'Commit MoveCommand, finish wire, commit selection'],
        ['dblclick', 'mouseDoubleClickEvent',   'step',     'Open IC sub-circuit, toggle input'],
        ['add',      'AddItems',                'step',     'Drop/paste elements'],
        ['del',      'DeleteItems',             'step',     'Delete selection'],
        ['mv',       'Move',                    'step',     'Drag elements'],
        ['rot',      'Rotate / Flip',           'step',     'Transform elements'],
        ['upd',      'Update',                  'step',     'Property changes (label, color, freq)'],
        ['split',    'Split',                   'step',     'Insert node into wire'],
        ['morph',    'Morph',                   'step',     'Change element type'],
        ['chgsz',    'ChangeInputSize',         'step',     'Add/remove ports'],
        ['stale',    'setCircuitUpdateRequired()', 'key',   'Marks simulation as stale \u2192 m_initialized = false. Next timer tick rebuilds sim graph.'],
      ],
  edges: [
        ['press',    'move'],
        ['move',     'release'],
        ['release',  'add'],
        ['release',  'mv'],
        ['dblclick', 'add'],
        ['press',    'del'],
        ['release',  'rot'],
        ['release',  'upd'],
        ['release',  'split'],
        ['release',  'morph'],
        ['release',  'chgsz'],
        ['add',      'stale'],
        ['del',      'stale'],
        ['mv',       'stale'],
        ['rot',      'stale'],
        ['upd',      'stale'],
        ['split',    'stale'],
        ['morph',    'stale'],
        ['chgsz',    'stale'],
      ]
};

