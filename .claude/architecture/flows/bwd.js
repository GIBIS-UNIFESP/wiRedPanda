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
        ['create', 'Create editor\nlinked to circuit',             'step',    ''],
        ['wave',    'Load or create\nwaveform data',             'key',     ''],
        ['load_e',  'Collect I/O elements\nfrom circuit',        'step',    ''],
        ['d_empty', 'Circuit has\nelements?',                    'decision',''],
        ['ex_emp',  'Error:\n"Circuit is empty"',               'error',  ''],
        ['d_in',    'Has input\nelements?',                      'decision',''],
        ['ex_in',   'Error:\n"No input elements"',              'error',  ''],
        ['d_out',   'Has output\nelements?',                     'decision',''],
        ['ex_out',  'Error:\n"No output elements"',             'error',  ''],
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
        ['d_empty', 'd_in',     'Yes'],
        ['d_in',    'ex_in',    'No'],
        ['d_in',    'd_out',    'Yes'],
        ['d_out',   'ex_out',   'No'],
        ['d_out',   'sort',     'Yes'],
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
        ['update', 'Run one sim cycle', 'step', '', 'sim_ops'],
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
        ['done_l', 'Close file, associate\nto main window,\nupdate title', 'end', ''],
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
        ['p_render','Render scene\nto pixmap',                 'step',    ''],
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
        ['png',      'p_render'],
        ['p_render', 'p_save'],
        ['pdf',      'pd_setup'],
        ['pd_setup', 'pd_render'],
        ['pd_render','pd_done'],
      ]
};

