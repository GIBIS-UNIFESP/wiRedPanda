// Flow definitions: serialization
flowRegistry['ser_ops'] = {
  title: 'Serialization \u2014 File Format',
  nodes: [
    ['root',        'Serialization Engine',          'start', 'Versioned binary .panda format'],
    ['save',        'Save',                          'key', 'Write header + metadata + elements + connections', 'ser_serialize'],
    ['load',        'Load',                          'key', 'Detect format, read header, deserialize items', 'ser_deserialize'],
    ['context',     'Serialization Context',         'step', 'Port map, version, working directory, blob registry'],
  ],
  edges: [
    ['root',    'save'],
    ['root',    'load'],
    ['load',    'context'],
  ]
};

flowRegistry['ser_serialize'] = {
  title: 'Save (Serialize)',
  nodes: [
        ['start',    'Save to stream',                             'start',    ''],
        ['header',   'Write file header\n(magic "WPCF" + version)', 'step',   ''],
        ['dolphin',  'Write waveform filename\n+ viewport rect',   'step',     ''],
        ['metadata', 'Write metadata (V4.5+)',                     'step',     'Includes embedded IC blob registry'],
        ['ids',      'Assign temp IDs\nto unsaved elements',       'step',     'Auto-incremented local IDs'],
        ['guard',    'Set up auto-restore\nfor original IDs',      'step',     'IDs restored on scope exit even if exception'],
        ['elms',     'Write all elements:\ntype tag + element data', 'key', '', 'ge_save'],
        ['conns',    'Write all connections:\ntype tag + wire data', 'key', ''],
        ['done',     'Done\n(IDs auto-restored)',                  'end',      ''],
      ],
  edges: [
        ['start',    'header'],
        ['header',   'dolphin'],
        ['dolphin',  'metadata'],
        ['metadata', 'ids'],
        ['ids',      'guard'],
        ['guard',    'elms'],
        ['elms',     'conns'],
        ['conns',    'done'],
      ]
};

flowRegistry['ser_deserialize'] = {
  title: 'Load (Deserialize)',
  nodes: [
        ['start',   'Load from stream',                            'start',    ''],
        ['header',  'Detect file format\nand read header',         'key',      'Try magic header, then legacy string, then headerless', 'ser_header'],
        ['preamble','Read preamble:\nwaveform + rect + metadata',  'step', ''],
        ['blobs',   'Extract embedded IC\nblobs from metadata',    'step',     ''],
        ['ctx',     'Build deserialization\ncontext',               'step',     'Port map, version, working directory'],
        ['loop',    'While stream\nhas data',                      'key',      ''],
        ['read_t',  'Read type tag',                               'step',     ''],
        ['d_status','Stream\nvalid?',                              'decision', ''],
        ['ex_s',    'Error:\n"Stream error"',                      'error',    ''],
        ['d_type',  'Item type?',                                  'decision', ''],
        ['elm',     'Element:\nread type, build,\nload from stream', 'key', '', 'ge_load'],
        ['conn',    'Connection:\ncreate, load,\nreconnect via port map', 'key', ''],
        ['ex_type', 'Error:\n"Invalid type tag"',                  'error',    ''],
        ['append',  'Add to item list',                            'step',     ''],
        ['done',    'Return item list',                            'end',      ''],
      ],
  edges: [
        ['start',    'header'],
        ['header',   'preamble'],
        ['preamble', 'blobs'],
        ['blobs',    'ctx'],
        ['ctx',      'loop'],
        ['loop',     'read_t'],
        ['read_t',   'd_status'],
        ['d_status', 'ex_s',      'No'],
        ['d_status', 'd_type',    'Yes'],
        ['d_type',   'elm',       'Element'],
        ['d_type',   'conn',      'Connection'],
        ['d_type',   'ex_type',   'unknown'],
        ['elm',      'append'],
        ['conn',     'append'],
        ['append',   'loop'],
        ['loop',     'done'],
      ]
};

flowRegistry['ser_header'] = {
  title: 'Read File Header',
  nodes: [
        ['start',   'Read file header',                         'start',    'Detects format: modern, legacy string, or headerless'],
        ['ver_qt',  'Set stream version',                       'step',     ''],
        ['save_p',  'Save stream position',                     'step',     ''],
        ['read_m',  'Read 4-byte\nmagic number',                'step',     ''],
        ['d_magic', 'Matches\nmagic header?',                   'decision', '"WPCF" = modern format'],
        ['mod_ver', 'Read version number',                      'end',      'Modern format (V4.5+)'],
        ['rewind1', 'Seek back to\nsaved position',             'step',     ''],
        ['read_s',  'Read app name string',                     'step',     ''],
        ['d_empty', 'String\nempty?',                           'decision', ''],
        ['rewind2', 'Seek back to\nsaved position',             'step',     ''],
        ['read_pt', 'Read center point',                        'step',     ''],
        ['d_null',  'Center\nis null?',                         'decision', ''],
        ['ex_inv',  'Error:\n"Invalid file format"',            'error', ''],
        ['rewind3', 'Seek back to\nsaved position',             'step',     ''],
        ['v41',     'Version = 4.1\n(last headerless release)', 'end',    ''],
        ['d_wp',    'Starts with\n"wiRedPanda"?',               'decision', ''],
        ['parse',   'Parse version from\nlegacy header string', 'end', 'Legacy string header (V4.2\u20134.4)'],
        ['ex_inv2', 'Error:\n"Invalid file format"',            'error', ''],
      ],
  edges: [
        ['start',   'ver_qt'],
        ['ver_qt',  'save_p'],
        ['save_p',  'read_m'],
        ['read_m',  'd_magic'],
        ['d_magic', 'mod_ver',   'Yes'],
        ['d_magic', 'rewind1',   'No'],
        ['rewind1', 'read_s'],
        ['read_s',  'd_empty'],
        ['d_empty', 'rewind2',   'Yes'],
        ['d_empty', 'd_wp',      'No'],
        ['rewind2', 'read_pt'],
        ['read_pt', 'd_null'],
        ['d_null',  'ex_inv',    'Yes'],
        ['d_null',  'rewind3',   'No'],
        ['rewind3', 'v41'],
        ['d_wp',    'parse',     'Yes'],
        ['d_wp',    'ex_inv2',   'No'],
      ]
};

flowRegistry['io_mod'] = {
  title: 'IO / Serialization',
  nodes: [
    ['f0', 'Save', 'key', '', 'ser_serialize'],
    ['f1', 'Load', 'key', '', 'ser_deserialize'],
  ],
  edges: [
    ['f0', 'f1'],
  ]
};

flowRegistry['io_save'] = {
  title: 'Save Flow',
  nodes: [
        ['start',    'Workspace save',                           'start',    ''],
        ['save', 'Serialize to stream', 'key', '', 'ser_serialize'],
      ],
  edges: [
        ['start',    'save'],
      ]
};

flowRegistry['io_load'] = {
  title: 'Load Flow',
  nodes: [
        ['start',    'Workspace load',                           'start',    ''],
        ['load', 'Deserialize from stream', 'key', '', 'ser_deserialize'],
        ['add',      'Add items to scene',                       'end',     'Register IDs, restore connections, trigger simulation rebuild'],
      ],
  edges: [
        ['start',    'load'],
        ['load',     'add'],
      ]
};

flowRegistry['io_context'] = {
  title: 'Serialization Context',
  nodes: [
        ['ctx',      'Per-load state',                           'key',     'Port map: serial ID \u2192 port. Version: file format version. Working directory. Blob registry: embedded IC blobs.'],
      ],
  edges: []
};
