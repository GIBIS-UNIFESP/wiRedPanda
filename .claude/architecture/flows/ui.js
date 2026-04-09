// Flow definitions: ui
flowRegistry['ui_view_ops'] = {
  title: 'GraphicsView \u2014 Viewport Controls',
  nodes: [
    ['f0', 'Zoom', 'key', '', 'ui_zoom'],
    ['f1', 'Context Menu', 'key', '', 'ui_ctx_menu']
  ],
  edges: [
    ['f0', 'f1']
  ]
};

flowRegistry['ui_zoom'] = {
  title: 'Zoom',
  nodes: [
        ['zoom_in', 'zoomIn()',                               'start',   'Ctrl+='],
        ['scale_in','scale(1.25, 1.25)\n++m_zoomLevel',      'step',    ''],
        ['emit_in', 'emit zoomChanged()',                     'end',     ''],
        ['zoom_out','zoomOut()',                               'start',   'Ctrl+-'],
        ['scale_out','scale(0.8, 0.8)\n--m_zoomLevel',       'step',    ''],
        ['emit_out','emit zoomChanged()',                     'end',     ''],
        ['reset',   'resetZoom()',                             'start',   'Ctrl+0'],
        ['rst_tf',  'resetTransform()\nm_zoomLevel = 0',     'step',    ''],
        ['emit_rst','emit zoomChanged()',                     'end',     ''],
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
        ['press',   'mousePressEvent\n(RightButton)',         'step',    ''],
        ['ctx',     'Scene::contextMenu\n(screenPos)',        'key',     ''],
        ['menu',    'ElementContextMenu::exec()',             'step',    'Build QMenu with context-sensitive actions'],
        ['d_act',   'User selected\nan action?',              'decision',''],
        ['no_act',  'Menu dismissed',                         'end',     ''],
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

flowRegistry['ui_palette_ops'] = {
  title: 'ElementPalette \u2014 Component Sidebar',
  nodes: [
    ['f0', 'Populate', 'key', '', 'ui_palette'],
    ['f1', 'Drag to Create', 'key', '', 'ui_palette_drag']
  ],
  edges: [
    ['f0', 'f1']
  ]
};

flowRegistry['ui_palette'] = {
  title: 'Populate',
  nodes: [
        ['start',   'ElementPalette::populate()',             'start',   ''],
        ['icons',   'setupTabIcons()',                        'step',    ''],
        ['tabs',    'For each category tab:\npopulateMenu(names, layout)','key',''],
        ['factory', 'For each name:\nElementFactory::textToType','step', ''],
        ['pixmap',  'ElementFactory::pixmap(type)',           'step',    ''],
        ['label',   'new ElementLabel\n(pixmap, type, name)', 'step',   'Creates draggable label widget'],
        ['search',  'Also add to\nsearch panel',              'step',    'Duplicate label in search tab for unified search'],
        ['done',    'setCurrentIndex(io tab)',                 'end',     ''],
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
        ['start',   'User drags ElementLabel',                'start',   ''],
        ['mime',    'Create QMimeData with\ntype + icFileName + offset','key',''],
        ['drag',    'QDrag::exec()',                           'step',    ''],
        ['drop',    'Scene::dropEvent()\nreceives the drop',   'key',     ''],
        ['build',   'buildElement(type)\n+ loadFromDrop()',    'step',    ''],
        ['cmd',     'AddItemsCommand\n\u2192 undoStack',       'end',    ''],
      ],
  edges: [
        ['start', 'mime'],
        ['mime',  'drag'],
        ['drag',  'drop'],
        ['drop',  'build'],
        ['build', 'cmd'],
      ]
};

flowRegistry['ui_menu_ops'] = {
  title: 'ElementContextMenu \u2014 Right-Click Actions',
  nodes: [
    ['f0', 'Menu Dispatch', 'key', '', 'ui_menu_dispatch']
  ],
  edges: [

  ]
};

flowRegistry['ui_menu_dispatch'] = {
  title: 'Menu Dispatch',
  nodes: [
        ['start',   'ElementContextMenu::exec()',             'start',   ''],
        ['build',   'Build QMenu from\nelement capabilities', 'key',     ''],
        ['d_label', 'hasLabel?',                              'decision',''],
        ['rename',  'Add "Rename"',                           'step',    ''],
        ['d_morph', 'canMorph?',                              'decision',''],
        ['morph',   'Add "Morph to..."\nsubmenu by group',    'step',    'Gate \u2192 other gate types, Memory \u2192 other FF types, etc.'],
        ['d_color', 'hasColors?',                             'decision',''],
        ['color',   'Add "Change color..."\nsubmenu',         'step',    ''],
        ['std',     'Add standard actions:\nRotate L/R, Copy,\nCut, Delete', 'step', ''],
        ['exec',    'menu.exec(screenPos)',                    'key',     ''],
        ['d_act',   'Action\nselected?',                      'decision',''],
        ['no_act',  'return\n(dismissed)',                     'error',   ''],
        ['match',   'Match action:\nRename \u2192 onRename()\nRotate \u2192 RotateCommand\nMorph \u2192 MorphCommand\nColor \u2192 colorCombo.set', 'end', ''],
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

flowRegistry['ui_theme_ops'] = {
  title: 'ThemeManager \u2014 Theme System',
  nodes: [
    ['f0', 'setTheme()', 'key', '', 'ui_theme']
  ],
  edges: [

  ]
};

flowRegistry['ui_theme'] = {
  title: 'setTheme()',
  nodes: [
        ['start',   'ThemeManager::setTheme(theme)',          'start',   'Light / Dark / System'],
        ['d_sys',   'theme ==\nSystem?',                      'decision',''],
        ['resolve', 'resolveSystemTheme()',                   'step',    'Query OS via SystemThemeDetector'],
        ['attrs',   'ThemeAttributes::\nsetTheme(effective)', 'key',     ''],
        ['d_light', 'Light or\nDark?',                       'decision', ''],
        ['light',   'Set light palette:\nwarm bg, dark dots,\nred selection, green wires', 'step', ''],
        ['dark',    'Set dark palette:\ngray bg, light dots,\nyellow selection, teal wires', 'step', ''],
        ['ports',   'Update port colors\nto match connection colors','step',''],
        ['d_same',  'Same theme\nas before?',                 'decision',''],
        ['r_same',  'return (no-op)',                          'end',     ''],
        ['persist', 'Settings::setTheme(theme)',              'step',    'Persist to disk'],
        ['emit',    'emit themeChanged()',                     'end',     'All Scene/Element/Connection instances repaint'],
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

flowRegistry['ui_mod'] = {
  title: 'UI Layer',
  nodes: [
    ['f0', 'Layout & Panels', 'key', '', 'ui_layout']
  ],
  edges: [

  ]
};

flowRegistry['ui_layout'] = {
  title: 'Layout & Panels',
  nodes: [
        ['mw',       'MainWindow',              'start',    ''],
        ['palette',  'ElementPalette\n(left sidebar)', 'step', 'ElementTabNavigator tabs. Populates from ElementFactory. Drag elements onto Scene.'],
        ['ws',       'Workspace\n(central, tabbed)', 'key',  'GraphicsView \u2192 Scene. One Workspace per open circuit. Tab bar for multiple files + inline ICs.'],
        ['editor',   'ElementEditor\n(right sidebar)', 'step', 'Property panel for selected element. PropertyDescriptor list. Generates UpdateCommand on change.'],
        ['file',     'File Ops',                'step',     'New / Open / Save / SaveAs. RecentFiles menu. FileUtils dialogs.'],
        ['sim',      'Sim Controls',            'step',     'Play / Pause / Reset. SimulationBlocker for modal ops.'],
        ['edit',     'Edit Ops',                'step',     'Undo / Redo / Copy / Cut / Paste. Rotate / Flip / Delete. SelectionCapabilities.'],
        ['export',   'Export',                  'step',     'CircuitExporter (PNG/PDF). ArduinoCodeGen. SystemVerilogCodeGen.'],
        ['bwd',      'BeWavedDolphin',          'step',     'Waveform editor window'],
        ['clock',    'ClockDialog',             'step',     'Configure clock frequency'],
        ['length',   'LengthDialog',            'step',     'Set simulation length'],
        ['dropzone', 'ICDropZone',              'step',     'Drag & drop .panda files'],
        ['lang',     'LanguageManager',         'step',     'i18n translations, locale switching'],
        ['theme',    'ThemeManager',            'step',     'Dark/Light themes, icon sets. SystemThemeDetector for OS detection.'],
        ['updchk',   'UpdateChecker',           'step',     'GitHub release API check'],
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

flowRegistry['core_mod'] = {
  title: 'Core Module',
  nodes: [
    ['f0', 'Application & Services', 'key', '', 'core_services']
  ],
  edges: [

  ]
};

flowRegistry['core_services'] = {
  title: 'Application & Services',
  nodes: [
        ['app',      'Application\n(singleton)',                 'start',    ''],
        ['settings', 'Settings',                                 'step',     'QSettings wrapper. Persistent user prefs: recent files, theme, language.'],
        ['theme',    'ThemeManager',                             'step',     'Dark/Light palettes. Icon set switching. \u2190 SystemThemeDetector (OS query).'],
        ['sentry',   'SentryHelpers',                            'step',     'Crash reporting. Breadcrumbs for debugging. \u2192 Sentry SDK (Crashpad/Breakpad).'],
        ['updchk',   'UpdateChecker',                            'step',     'GitHub release API. Notifies user of new versions.'],
        ['enums',    'Enums',                                    'step',     'ElementType (40+), ElementGroup, Status (Active/Inactive/Unknown/Error)'],
        ['itemid',   'ItemWithId',                               'step',     'Base class: stable numeric ID. Used by GraphicElement + QNEConnection. Scene-scoped.'],
        ['common',   'Common',                                   'step',     'Shared helpers, logging categories, PANDACEPTION macro'],
        ['prio',     'Priorities',                               'step',     'Constants for topological sort ordering'],
        ['statops',  'StatusOps',                                'step',     'Tristate logic: AND/OR/NOT with Unknown handling'],
      ],
  edges: [
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

