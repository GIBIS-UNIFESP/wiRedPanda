// Flow definitions: ui
flowRegistry['ui_zoom'] = {
  title: 'Zoom',
  nodes: [
        ['zoom_in', 'Zoom In',                               'start',   'Ctrl+='],
        ['scale_in','Scale up 125%,\nincrement zoom level',  'step',    ''],
        ['emit_in', 'Notify zoom changed',                   'end',     ''],
        ['zoom_out','Zoom Out',                               'start',   'Ctrl+-'],
        ['scale_out','Scale down 80%,\ndecrement zoom level','step',    ''],
        ['emit_out','Notify zoom changed',                   'end',     ''],
        ['reset',   'Reset Zoom',                             'start',   'Ctrl+0'],
        ['rst_tf',  'Reset transform,\nclear zoom level',    'step',    ''],
        ['emit_rst','Notify zoom changed',                   'end',     ''],
      ],
  edges: [
        ['zoom_in',  'scale_in'],
        ['scale_in', 'emit_in'],
        ['zoom_out', 'scale_out'],
        ['scale_out','emit_out'],
        ['reset',    'rst_tf'],
        ['rst_tf',   'emit_rst'],
      ]
};

flowRegistry['ui_ctx_menu'] = {
  title: 'Context Menu',
  nodes: [
        ['start',   'Right-click on canvas',                  'start',   ''],
        ['press',   'Mouse press\n(right button)',             'step',    ''],
        ['ctx',     'Scene: open\ncontext menu',               'key',     ''],
        ['menu', 'Build and show\ncontext menu', 'step', 'Context-sensitive actions', 'ui_menu_dispatch'],
        ['d_act',   'User selected\nan action?',              'decision',''],
        ['no_act',  'Menu dismissed',                          'end',     ''],
        ['do_act',  'Execute action\n(rotate, flip, morph, etc.)', 'end',''],
      ],
  edges: [
        ['start',  'press'],
        ['press',  'ctx'],
        ['ctx',    'menu'],
        ['menu',   'd_act'],
        ['d_act',  'no_act',  'No'],
        ['d_act',  'do_act',  'Yes'],
      ]
};

flowRegistry['ui_palette'] = {
  title: 'Populate',
  nodes: [
        ['start',   'Populate palette',                       'start',   ''],
        ['icons',   'Set up tab icons',                        'step',    ''],
        ['tabs',    'For each category tab:\npopulate with elements', 'key',''],
        ['factory', 'Convert name to\nelement type',           'step',    ''],
        ['pixmap',  'Get element icon',                        'step',    ''],
        ['label',   'Create draggable\nlabel widget',          'step',   ''],
        ['search',  'Also add to\nsearch panel',               'step',    'Duplicate label in search tab for unified search'],
        ['done',    'Select I/O tab',                           'end',     ''],
      ],
  edges: [
        ['start',   'icons'],
        ['icons',   'tabs'],
        ['tabs',    'factory'],
        ['factory', 'pixmap'],
        ['pixmap',  'label'],
        ['label',   'search'],
        ['search',  'done'],
      ]
};

flowRegistry['ui_palette_drag'] = {
  title: 'Drag to Create',
  nodes: [
        ['start',   'User drags element\nfrom palette',       'start',   ''],
        ['mime',    'Create drag data with\ntype + IC filename + offset','key',''],
        ['drag',    'Start drag operation',                    'step',    ''],
        ['drop', 'Scene receives\nthe drop', 'key', '', 'scene_drop'],
        ['build', 'Build element\nand load IC file', 'step', '', 'ef_build'],
        ['cmd', 'Push add command\nto undo stack', 'end', '', 'cmd_add'],
      ],
  edges: [
        ['start', 'mime'],
        ['mime',  'drag'],
        ['drag',  'drop'],
        ['drop',  'build'],
        ['build', 'cmd'],
      ]
};

flowRegistry['ui_menu_dispatch'] = {
  title: 'Menu Dispatch',
  nodes: [
        ['start',   'Build context menu',                     'start',   ''],
        ['build',   'Build menu from\nelement capabilities',   'key',     ''],
        ['d_label', 'Has label?',                              'decision',''],
        ['rename',  'Add "Rename"',                            'step',    ''],
        ['d_morph', 'Can morph?',                              'decision',''],
        ['morph',   'Add "Morph to..."\nsubmenu by group',     'step',    'Gate \u2192 other gate types, Memory \u2192 other FF types, etc.'],
        ['d_color', 'Has colors?',                             'decision',''],
        ['color',   'Add "Change color..."\nsubmenu',          'step',    ''],
        ['std',     'Add standard actions:\nRotate L/R, Copy,\nCut, Delete', 'step', ''],
        ['exec',    'Show menu',                                'key',     ''],
        ['d_act',   'Action\nselected?',                       'decision',''],
        ['no_act',  'Dismissed',                                'error',   ''],
        ['match',   'Execute action:\nRename, Rotate,\nMorph, Color, etc.', 'end', ''],
      ],
  edges: [
        ['start',   'build'],
        ['build',   'd_label'],
        ['d_label', 'rename',   'Yes'],
        ['d_label', 'd_morph',  'No'],
        ['rename',  'd_morph'],
        ['d_morph', 'morph',    'Yes'],
        ['d_morph', 'd_color',  'No'],
        ['morph',   'd_color'],
        ['d_color', 'color',    'Yes'],
        ['d_color', 'std',      'No'],
        ['color',   'std'],
        ['std',     'exec'],
        ['exec',    'd_act'],
        ['d_act',   'no_act',   'No'],
        ['d_act',   'match',    'Yes'],
      ]
};

flowRegistry['ui_theme'] = {
  title: 'Set Theme',
  nodes: [
        ['start',   'Set theme\n(Light / Dark / System)',     'start',   ''],
        ['d_sys',   'System\ntheme?',                          'decision',''],
        ['resolve', 'Query OS\nfor current theme',             'step',    ''],
        ['attrs',   'Apply theme\nattributes',                  'key',     ''],
        ['d_light', 'Light or\nDark?',                         'decision', ''],
        ['light',   'Set light palette:\nwarm bg, dark dots,\nred selection, green wires', 'step', ''],
        ['dark',    'Set dark palette:\ngray bg, light dots,\nyellow selection, teal wires', 'step', ''],
        ['ports',   'Update port colors\nto match wire colors', 'step',''],
        ['d_same',  'Same theme\nas before?',                   'decision',''],
        ['r_same',  'No-op',                                    'end',     ''],
        ['persist', 'Persist to settings',                      'step',    ''],
        ['emit',    'Notify theme changed',                     'end',     'All scenes, elements, and connections repaint'],
      ],
  edges: [
        ['start',   'd_sys'],
        ['d_sys',   'resolve',  'Yes'],
        ['d_sys',   'attrs',    'No'],
        ['resolve', 'attrs'],
        ['attrs',   'd_light'],
        ['d_light', 'light',    'Light'],
        ['d_light', 'dark',     'Dark'],
        ['light',   'ports'],
        ['dark',    'ports'],
        ['ports',   'd_same'],
        ['d_same',  'r_same',   'Yes'],
        ['d_same',  'persist',  'No'],
        ['persist', 'emit'],
      ]
};

flowRegistry['ui_layout'] = {
  title: 'Layout & Panels',
  nodes: [
        ['mw',       'MainWindow',              'start',    ''],
        ['palette', 'Element Palette\n(left sidebar)', 'step', 'Tab navigator. Populates from factory. Drag elements onto scene.', 'ui_palette'],
        ['ws',       'Workspace\n(central, tabbed)', 'key',  'Graphics view \u2192 scene. One workspace per open circuit. Tab bar for multiple files + inline ICs.'],
        ['editor', 'Element Editor\n(right sidebar)', 'step', 'Property panel for selected element. Generates update command on change.', 'cmd_update'],
        ['file',     'File Ops',                'step',     'New / Open / Save / SaveAs. Recent files menu.'],
        ['sim',      'Sim Controls',            'step',     'Play / Pause / Reset. Simulation blocker for modal ops.'],
        ['edit',     'Edit Ops',                'step',     'Undo / Redo / Copy / Cut / Paste. Rotate / Flip / Delete.'],
        ['export',   'Export',                  'step',     'Image exporter (PNG/PDF). Arduino. SystemVerilog.'],
        ['bwd', 'BeWavedDolphin', 'step', 'Waveform editor window', 'bwd_ops'],
        ['clock',    'Clock Dialog',            'step',     'Configure clock frequency'],
        ['length',   'Length Dialog',           'step',     'Set simulation length'],
        ['dropzone', 'IC DropZone',             'step',     'Drag & drop .panda files'],
        ['lang',     'Language Manager',        'step',     'i18n translations, locale switching'],
        ['theme', 'Theme Manager', 'step', 'Dark/Light themes, icon sets. OS theme detection.', 'ui_theme'],
        ['updchk',   'Update Checker',          'step',     'GitHub release API check'],
      ],
  edges: [
        ['mw', 'palette',  'composes'],
        ['mw', 'ws',       'composes'],
        ['mw', 'editor',   'composes'],
        ['mw', 'file',     'toolbar'],
        ['mw', 'sim',      'toolbar'],
        ['mw', 'edit',     'toolbar'],
        ['mw', 'export',   'toolbar'],
        ['mw', 'bwd',      'opens'],
        ['mw', 'clock',    'opens'],
        ['mw', 'length',   'opens'],
        ['mw', 'dropzone', 'opens'],
        ['mw', 'lang',     'service'],
        ['mw', 'theme',    'service'],
        ['mw', 'updchk',   'service'],
      ]
};

flowRegistry['core_services'] = {
  title: 'Application & Services',
  nodes: [
    ['app',      'Application\n(singleton)',   'start', 'Program entry creates this; holds global state'],
    ['mw',       'MainWindow',                 'key',   'Top-level window with tabs, menus, toolbars', 'mw_ops'],
    ['settings', 'Settings',                   'step',  'Persistent user prefs: recent files, theme, language.'],
    ['theme',    'Theme Manager',              'key',   'Dark/Light/System theme switching', 'ui_theme'],
    ['sentry',   'Sentry Helpers',             'step',  'Crash reporting. Breadcrumbs for debugging.'],
    ['updchk',   'Update Checker',             'step',  'GitHub release API. Notifies user of new versions.'],
    ['enums',    'Enums',                      'step',  'ElementType (40+), ElementGroup, Status (Active/Inactive/Unknown/Error)'],
    ['itemid',   'ItemWithId',                 'step',  'Base class: stable numeric ID. Used by elements + connections. Scene-scoped.'],
    ['common',   'Common',                     'step',  'Shared helpers, logging categories, error macros'],
    ['prio',     'Priorities',                 'key',   'Constants for topological sort ordering', 'sim_topo_sort'],
    ['statops',  'StatusOps',                  'step',  'Tristate logic: AND/OR/NOT with Unknown handling'],
  ],
  edges: [
    ['app',     'mw',       'creates'],
    ['app',     'settings', 'manages'],
    ['app',     'theme',    'manages'],
    ['app',     'sentry',   'manages'],
    ['app',     'updchk',   'manages'],
    ['app',     'enums',    'shared types'],
    ['app',     'itemid',   'shared types'],
    ['app',     'common',   'shared types'],
    ['enums',   'prio'],
    ['enums',   'statops'],
  ]
};
