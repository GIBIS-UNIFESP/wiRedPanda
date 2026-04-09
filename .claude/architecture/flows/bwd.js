// Flow definitions: bwd
flowRegistry['bwd_ops'] = {
  title: 'BeWavedDolphin \u2014 Waveform Editor',
  nodes: [
    ['root',   'BeWavedDolphin',                      'start', ''],
    ['open',   'Open Editor\n(loadElements + table)',  'key', 'Collect I/O from circuit, build SignalModel', 'bwd_open'],
    ['run',    'Run Simulation\n(per-timestep loop)',   'key', 'Set inputs \u2192 sim.update() \u2192 read outputs', 'bwd_run'],
    ['save',   'Save / Load\n(.dolphin / .csv)',        'key', 'Binary or CSV format', 'bwd_save_load'],
    ['export', 'Export\n(PNG / PDF)',                    'key', 'Render scene to image or printer', 'bwd_export'],
  ],
  edges: [
    ['root', 'open'],
    ['open', 'run'],
    ['root', 'save'],
    ['root', 'export'],
  ]
};

flowRegistry['bwd_open'] = {
  title: 'Open Editor',
  nodes: [
        ['start',   'MainWindow \u2192\non_actionWaveform',  'start',   ''],
        ['create', 'new BewavedDolphin\n(scene, true, this)', 'step', '', 'bwd_ops'],
        ['wave',    'createWaveform(dolphinFile)',            'key',     ''],
        ['load_e',  'loadElements()',                         'step',    'Collect Input/Output elements from circuit scene'],
        ['d_empty', 'elements\nempty?',                      'decision',''],
        ['ex_emp',  'throw PANDACEPTION\n"Circuit is empty"', 'error',  ''],
        ['d_io',    'Has inputs\nAND outputs?',              'decision',''],
        ['ex_io',   'throw PANDACEPTION\n"No I/O elements"', 'error',  ''],
        ['sort',    'Sort I/O by label\n(case-insensitive)',  'step',   ''],
        ['table',   'loadNewTable()',                         'step',    'Build SignalModel: rows = signals, cols = timesteps'],
        ['run',     'run()',                                   'key',    'Simulate all timesteps'],
        ['show',    'show()',                                   'end',   ''],
      ],
  edges: [
        ['start',   'create'],
        ['create',  'wave'],
        ['wave',    'load_e'],
        ['load_e',  'd_empty'],
        ['d_empty', 'ex_emp',   'Yes'],
        ['d_empty', 'd_io',     'No'],
        ['d_io',    'ex_io',    'No'],
        ['d_io',    'sort',     'Yes'],
        ['sort',    'table'],
        ['table',   'run'],
        ['run',     'show'],
      ]
};

flowRegistry['bwd_run'] = {
  title: 'Run Simulation',
  nodes: [
        ['start',   'run()',                                  'start',   ''],
        ['block',   'SimulationBlocker\npause simulation',   'step',    ''],
        ['loop',    'For column = 0\nto columnCount-1',       'key',     ''],
        ['set_in',  'Set circuit inputs\nfrom column values', 'step',    'For each input element/port: setOn(value)'],
        ['update', 'simulation->update()', 'step', 'Run one sim cycle', 'sim_cycle'],
        ['read_out','Read circuit outputs\n\u2192 fill output cells','step','For each output port: status \u2192 createElement()'],
        ['next',    'Next column',                            'step',    ''],
        ['restore', 'restoreInputs()',                        'end',     ''],
      ],
  edges: [
        ['start',   'block'],
        ['block',   'loop'],
        ['loop',    'set_in'],
        ['set_in',  'update'],
        ['update',  'read_out'],
        ['read_out','next'],
        ['next',    'loop'],
        ['loop',    'restore'],
      ]
};

flowRegistry['bwd_save_load'] = {
  title: 'Save / Load .dolphin',
  nodes: [
        ['save_s',  'on_actionSave_triggered()',              'start',   ''],
        ['d_file',  'currentFile\nempty?',                   'decision', ''],
        ['save_as', 'on_actionSaveAs()\n\u2192 FileDialog',  'step',    ''],
        ['do_save', 'save(filePath)',                          'key',     'writeDolphinHeader + serialize table data'],
        ['done_s',  'statusBar: "Saved"',                     'end',     ''],
        ['load_s',  'on_actionLoad_triggered()',               'start',   ''],
        ['dlg',     'FileDialog::getOpenFileName\nfilter: *.dolphin *.csv','step',''],
        ['d_empty', 'fileName\nempty?',                       'decision', ''],
        ['r_empty', 'return',                                  'error',   ''],
        ['do_load', 'load(fileName)',                           'key',     ''],
        ['d_exist', 'File exists?',                            'decision', ''],
        ['ex_nf',   'throw PANDACEPTION',                      'error',   ''],
        ['d_open',  'File readable?',                          'decision', ''],
        ['ex_open', 'throw PANDACEPTION',                      'error',   ''],
        ['d_fmt',   'Format?',                                 'decision', ''],
        ['dol',     '.dolphin:\nreadDolphinHeader \u2192\nload(stream)','step',''],
        ['csv',     '.csv:\nload(file) as CSV',                'step',    ''],
        ['ex_fmt',  'throw PANDACEPTION\n"Format not supported"','error', ''],
        ['done_l', 'setWindowTitle\n"beWavedDolphin [file]"', 'end', '', 'bwd_ops'],
      ],
  edges: [
        ['save_s',  'd_file'],
        ['d_file',  'save_as',  'Yes'],
        ['d_file',  'do_save',  'No'],
        ['save_as', 'do_save'],
        ['do_save', 'done_s'],
        ['load_s',  'dlg'],
        ['dlg',     'd_empty'],
        ['d_empty', 'r_empty',  'Yes'],
        ['d_empty', 'do_load',  'No'],
        ['do_load', 'd_exist'],
        ['d_exist', 'ex_nf',    'No'],
        ['d_exist', 'd_open',   'Yes'],
        ['d_open',  'ex_open',  'No'],
        ['d_open',  'd_fmt',    'Yes'],
        ['d_fmt',   'dol',      '.dolphin'],
        ['d_fmt',   'csv',      '.csv'],
        ['d_fmt',   'ex_fmt',   'other'],
        ['dol',     'done_l'],
        ['csv',     'done_l'],
      ]
};

flowRegistry['bwd_export'] = {
  title: 'Export',
  nodes: [
        ['start',   'Export menu action',                      'start',   ''],
        ['d_fmt',   'Format?',                                 'decision', ''],
        ['png',     'exportToPng(filename)',                    'key',     ''],
        ['p_size',  'setTableViewSize\n(fit all cells)',       'step',    ''],
        ['p_rect',  'setSceneRect(itemsBounds)',               'step',    ''],
        ['p_render','Render scene to\nQPixmap \u2192 QImage', 'step',    ''],
        ['p_save',  'image.save(filename)',                    'end',      ''],
        ['pdf',     'exportToPdf(pdfFile)',                     'key',     ''],
        ['pd_setup','QPrinter: A4, Landscape,\nHighResolution','step',   ''],
        ['pd_render','scene->render(painter)',                 'step',    ''],
        ['pd_done', 'painter.end()',                           'end',      ''],
      ],
  edges: [
        ['start',    'd_fmt'],
        ['d_fmt',    'png',       'PNG'],
        ['d_fmt',    'pdf',       'PDF'],
        ['png',      'p_size'],
        ['p_size',   'p_rect'],
        ['p_rect',   'p_render'],
        ['p_render', 'p_save'],
        ['pdf',      'pd_setup'],
        ['pd_setup', 'pd_render'],
        ['pd_render','pd_done'],
      ]
};

flowRegistry['bwd_pipeline'] = {
  title: 'Waveform Pipeline',
  nodes: [
        ['start',    'MainWindow opens\nBeWavedDolphin(scene)',  'start',    ''],
        ['load',     'loadElements()',                           'step',     'Collect Input/Output GraphicElements from circuit scene'],
        ['create',   'createWaveform()',                         'step',     'Build table: rows = I/O signals, columns = time steps (default 32)'],
        ['model',    'SignalModel\n(QStandardItemModel)',        'key',      'Rows: input signals (editable) + output signals (read-only). Columns: simulation time steps. Cells: 0/1 values.'],
        ['run',      'run()',                                    'key',      'For each time step: 1. Set circuit inputs to column values 2. simulation\u2192update() 3. Read outputs \u2192 fill cells'],
        ['delegate', 'SignalDelegate',                           'step',     'Custom cell rendering: waveform lines or numeric values'],
        ['view',     'WaveformView',                             'step',     'QGraphicsView display. Zoom, pan, fit-to-screen.'],
        ['save',     'Save .dolphin',                            'step',     'Magic: 0x57505746 ("WPWF"). Binary QDataStream of table data.'],
        ['load_d',   'Load .dolphin',                            'step',     'Validate header \u2192 fill SignalModel. Check signal count matches current circuit.'],
        ['exp',      'Export',                                   'end',      'PNG / PDF / plain text'],
      ],
  edges: [
        ['start',    'load'],
        ['load',     'create'],
        ['create',   'model'],
        ['model',    'run'],
        ['run',      'delegate'],
        ['run',      'view'],
        ['model',    'save'],
        ['model',    'load_d'],
        ['model',    'exp'],
      ]
};

