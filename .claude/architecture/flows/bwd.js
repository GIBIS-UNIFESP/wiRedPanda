// Flow definitions: bwd
flowRegistry['bwd_ops'] = {
  title: 'BeWavedDolphin \u2014 Waveform Editor',
  nodes: [
    ['root',   'BeWavedDolphin',                      'start', ''],
    ['open',   'Open Editor\n(load elements + table)',  'key', 'Collect I/O from circuit, build signal model', 'bwd_open'],
    ['run',    'Run Simulation\n(per-timestep loop)',   'key', 'Set inputs \u2192 run update \u2192 read outputs', 'bwd_run'],
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
        ['start',   'Open waveform editor',                     'start',   ''],
        ['create', 'Create editor\nlinked to circuit', 'step', '', 'bwd_ops'],
        ['wave',    'Load or create\nwaveform data',             'key',     ''],
        ['load_e',  'Collect I/O elements\nfrom circuit',        'step',    ''],
        ['d_empty', 'Circuit has\nelements?',                    'decision',''],
        ['ex_emp',  'Error:\n"Circuit is empty"',               'error',  ''],
        ['d_io',    'Has inputs\nAND outputs?',                  'decision',''],
        ['ex_io',   'Error:\n"No I/O elements"',                'error',  ''],
        ['sort',    'Sort I/O by label\n(case-insensitive)',     'step',   ''],
        ['table',   'Build signal table:\nrows = signals,\ncols = timesteps', 'step', ''],
        ['run',     'Run simulation\nfor all timesteps',         'key',    ''],
        ['show',    'Show editor window',                        'end',   ''],
      ],
  edges: [
        ['start',   'create'],
        ['create',  'wave'],
        ['wave',    'load_e'],
        ['load_e',  'd_empty'],
        ['d_empty', 'ex_emp',   'No'],
        ['d_empty', 'd_io',     'Yes'],
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
        ['start',   'Run simulation',                            'start',   ''],
        ['block',   'Pause circuit\nsimulation',                 'step',    ''],
        ['loop',    'For each\ntimestep column',                  'key',     ''],
        ['set_in',  'Set circuit inputs\nfrom column values',    'step',    ''],
        ['update', 'Run one sim cycle', 'step', '', 'sim_cycle'],
        ['read_out','Read circuit outputs\n\u2192 fill output cells','step',''],
        ['next',    'Next column',                                'step',    ''],
        ['restore', 'Restore original\ninput values',             'end',     ''],
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
        ['save_s',  'Save action',                               'start',   ''],
        ['d_file',  'File path\nassigned?',                      'decision', ''],
        ['save_as', 'Save As dialog',                             'step',    ''],
        ['do_save', 'Write header +\nserialize table data',       'key',     ''],
        ['done_s',  'Status: "Saved"',                            'end',     ''],
        ['load_s',  'Load action',                                'start',   ''],
        ['dlg',     'Open file dialog\n(*.dolphin, *.csv)',       'step',''],
        ['d_empty', 'File\nselected?',                            'decision', ''],
        ['r_empty', 'Abort',                                      'error',   ''],
        ['do_load', 'Load file',                                   'key',     ''],
        ['d_exist', 'File exists?',                                'decision', ''],
        ['ex_nf',   'Error:\n"File not found"',                   'error',   ''],
        ['d_open',  'File readable?',                              'decision', ''],
        ['ex_open', 'Error:\n"Cannot open file"',                 'error',   ''],
        ['d_fmt',   'Format?',                                     'decision', ''],
        ['dol',     '.dolphin:\nread header +\nload table data',  'step',''],
        ['csv',     '.csv:\nload as CSV',                          'step',    ''],
        ['ex_fmt',  'Error:\n"Format not supported"',             'error', ''],
        ['done_l', 'Update window title', 'end', '', 'bwd_ops'],
      ],
  edges: [
        ['save_s',  'd_file'],
        ['d_file',  'save_as',  'No'],
        ['d_file',  'do_save',  'Yes'],
        ['save_as', 'do_save'],
        ['do_save', 'done_s'],
        ['load_s',  'dlg'],
        ['dlg',     'd_empty'],
        ['d_empty', 'r_empty',  'No'],
        ['d_empty', 'do_load',  'Yes'],
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
        ['png',     'Export to PNG',                            'key',     ''],
        ['p_size',  'Resize table view\nto fit all cells',     'step',    ''],
        ['p_rect',  'Set scene bounds\nto item bounds',        'step',    ''],
        ['p_render','Render scene\nto image',                  'step',    ''],
        ['p_save',  'Save image to file',                      'end',      ''],
        ['pdf',     'Export to PDF',                            'key',     ''],
        ['pd_setup','Configure printer:\nA4, Landscape,\nHigh Resolution','step',''],
        ['pd_render','Render scene\nto printer',               'step',    ''],
        ['pd_done', 'Finish PDF',                              'end',      ''],
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
        ['start',    'MainWindow opens\nwaveform editor',           'start',    ''],
        ['load',     'Collect I/O elements\nfrom circuit',           'step',     ''],
        ['create',   'Build waveform table:\nrows = I/O signals,\ncols = timesteps',  'step', ''],
        ['model',    'Signal Model',                                 'key',      'Rows: input signals (editable) + output signals (read-only). Columns: simulation time steps. Cells: 0/1 values.'],
        ['run',      'Run simulation',                               'key',      'For each time step: 1. Set circuit inputs 2. Run sim cycle 3. Read outputs \u2192 fill cells'],
        ['delegate', 'Signal Delegate',                              'step',     'Custom cell rendering: waveform lines or numeric values'],
        ['view',     'Waveform View',                                'step',     'Display widget. Zoom, pan, fit-to-screen.'],
        ['save',     'Save .dolphin',                                'step',     'Binary format with header and table data'],
        ['load_d',   'Load .dolphin',                                'step',     'Validate header \u2192 fill signal model. Check signal count matches circuit.'],
        ['exp',      'Export',                                       'end',      'PNG / PDF / plain text'],
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
