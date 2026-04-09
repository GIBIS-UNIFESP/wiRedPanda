// Root overview flow
flowRegistry['overview'] = {
  title: 'wiRedPanda \u2014 Application Overview',
  nodes: [
    ['entry',  'Main.cpp\nApplication Startup', 'start', 'Program entry, Qt event loop, creates MainWindow'],
    ['mw',     'MainWindow\n(Tab Management)',   'key',   'Menus, toolbars, tab bar hosting WorkSpaces', 'mw_ops'],
    ['ws',     'Workspace\n(Per-Tab Circuit)',    'key',   'Scene + GraphicsView + Simulation per tab', 'ws_ops'],
    ['scene',  'Scene\n(Circuit Canvas)',         'key',   'QGraphicsScene: elements, wires, undo stack', 'scene_ops'],
    ['sim',    'Simulation\n(1ms Cycle)',         'key',   'Topological sort, 4-phase update, feedback settling', 'sim_ops'],
    ['serial', 'Serialization\n(.panda I/O)',     'key',   'Versioned binary QDataStream format', 'ser_ops'],
    ['elem',   'Elements\n(GraphicElement)',      'key',   'Gates, memory, I/O, factory, properties', 'ge_ops'],
    ['conn',   'Wiring\n(Ports + Connections)',   'key',   'QNEPort, QNEConnection, ConnectionManager', 'conn_ops'],
    ['ic',     'IC / Inline IC\n(Sub-Circuits)',  'key',   'File-backed + embedded, edit in tab', 'ic_ops'],
    ['cmd',    'Undo/Redo\n(Commands)',           'key',   'QUndoStack: Add, Delete, Move, Update', 'cmd_ops'],
    ['bwd',    'BeWavedDolphin\n(Waveforms)',     'key',   'Signal editor, simulation runner, export', 'bwd_ops'],
    ['cg',     'Code Generation\n(Arduino / SV)', 'key',  'Export circuits as C++ or HDL', 'cg_ops'],
    ['ui',     'UI Utilities\n(Theme, Palette)',  'key',   'Theme switching, element palette, zoom', 'ui_ops'],
  ],
  edges: [
    ['entry', 'mw'],
    ['mw',    'ws'],
    ['ws',    'scene'],
    ['ws',    'sim'],
    ['scene', 'elem'],
    ['scene', 'conn'],
    ['scene', 'cmd'],
    ['elem',  'ic'],
    ['ws',    'serial'],
    ['mw',    'bwd'],
    ['mw',    'cg'],
    ['mw',    'ui'],
  ]
};

// Code generation overview
flowRegistry['cg_ops'] = {
  title: 'Code Generation',
  nodes: [
    ['menu',    'MainWindow \u2192\nExport menu',       'start', ''],
    ['arduino', 'Arduino C++\nCode Generator',          'key',   'Maps circuit to digital I/O + setup()/loop()', 'cg_arduino'],
    ['verilog', 'SystemVerilog\nHDL Generator',         'key',   'Maps circuit to module with wire/reg declarations', 'cg_verilog'],
    ['shared',  'Shared Pipeline:\nsort by topology \u2192\nassign variables \u2192\nresolve wireless \u2192\nmap to language \u2192\nwrite file', 'step', ''],
  ],
  edges: [
    ['menu',    'arduino'],
    ['menu',    'verilog'],
    ['arduino', 'shared'],
    ['verilog', 'shared'],
  ]
};

// UI utilities overview
flowRegistry['ui_ops'] = {
  title: 'UI Utilities',
  nodes: [
    ['root',    'UI Systems',                    'start',  ''],
    ['theme',   'Theme Manager\n(Light/Dark/System)', 'key', 'ThemeAttributes, SystemThemeDetector, palette switching', 'ui_theme'],
    ['palette', 'Element Palette\n(Left Sidebar)',     'key', 'Category tabs, search, drag-to-create', 'ui_palette'],
    ['editor',  'Element Editor\n(Right Sidebar)',     'key', 'Property panel, selection sync', 'editor_apply'],
    ['view',    'Graphics View\n(Zoom / Pan)',         'key', 'Zoom in/out/reset, context menu dispatch', 'ui_zoom'],
    ['ctxmenu', 'Context Menu\n(Right-Click)',         'key', 'Capability-aware actions: rename, morph, rotate, etc.', 'ui_menu_dispatch'],
    ['drag',    'Drag \u2192 Drop\non Canvas',         'step','ElementLabel starts QDrag \u2192 Scene::dropEvent()', 'ui_palette_drag'],
  ],
  edges: [
    ['root',    'theme'],
    ['root',    'palette'],
    ['root',    'editor'],
    ['root',    'view'],
    ['view',    'ctxmenu'],
    ['palette', 'drag'],
  ]
};
