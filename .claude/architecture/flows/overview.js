// Root overview flow + module overviews
flowRegistry['overview'] = {
  title: 'wiRedPanda — Application Overview',
  nodes: [
    ['entry',  'Main.cpp\nApplication Startup', 'start', 'Program entry point, Qt event loop'],
    ['mw',     'MainWindow\n(Tab Management)',   'key',   'Top-level window hosting circuit tabs', 'mainwindow_ops'],
    ['ws',     'Workspace\n(Per-Tab Environment)','key',  'Scene + View + Simulation per tab', 'workspace_ops'],
    ['scene',  'Scene\n(Circuit Editing)',        'key',   'QGraphicsScene: elements, wires, undo', 'scene_ops'],
    ['sim',    'Simulation\n(1ms Cycle)',          'key',  'Topological sort, feedback settling', 'simulation_ops'],
    ['serial', 'Serialization\n(.panda Files)',    'key',  'Binary QDataStream format', 'serialization_ops'],
    ['elem',   'Element System\n(GraphicElement)', 'key',  'Gates, memory, I/O, IC', 'element_mod'],
    ['conn',   'Connections\n(Wiring)',             'key', 'Ports + wire creation/validation', 'nodes_mod'],
    ['ic',     'IC / Inline IC\n(Sub-Circuits)',    'key', 'Embed, extract, edit inline', 'ic_ops'],
    ['bwd',    'BeWavedDolphin\n(Waveform Editor)', 'key', 'Signal visualization + export', 'bwd_ops'],
    ['cg',     'Code Generation\n(Arduino + SV)',   'key', 'Export circuits as code', 'codegen_mod'],
    ['ui',     'UI Utilities\n(Theme, Palette, Zoom)','key','Supporting UI systems', 'ui_mod'],
    ['cmd',    'Undo/Redo System\n(Commands)',       'key', 'Add, Delete, Move, Update', 'commands_ops'],
  ],
  edges: [
    ['entry', 'mw'],
    ['mw',    'ws'],
    ['ws',    'scene'],
    ['ws',    'sim'],
    ['ws',    'serial'],
    ['scene', 'elem'],
    ['scene', 'conn'],
    ['scene', 'cmd'],
    ['elem',  'ic'],
    ['mw',    'bwd'],
    ['mw',    'cg'],
    ['mw',    'ui'],
  ]
};
