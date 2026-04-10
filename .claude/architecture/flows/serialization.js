// Flow definitions: serialization
flowRegistry['ser_ops'] = {
  title: 'Serialization \u2014 File Format',
  nodes: [
    ['root',     'Serialization Engine',               'start', 'Versioned binary format'],
    ['header',   'Read File Header\n(Format Detection)', 'key', 'Magic header vs legacy string vs headerless', 'ser_header'],
    ['serialize','Serialize\n(Write Items)',            'key', 'Assign temp IDs \u2192 write elements \u2192 write connections', 'ser_serialize'],
    ['deserialize','Deserialize\n(Read Items)',        'key', 'Loop: type tag \u2192 build element \u2192 load \u2192 register ports', 'ser_deserialize'],
    ['context',  'Serialization Context',              'step', 'Port map, version, working directory, blob registry'],
  ],
  edges: [
    ['root',      'header'],
    ['root',      'serialize'],
    ['root',      'deserialize'],
    ['deserialize','context'],
    ['header',     'context'],
  ]
};

flowRegistry['ser_serialize'] = {
  title: 'Serialize (Write Items)',
  nodes: [
        ['start',   'Serialize items\nto stream',                  'start',    ''],
        ['ids',     'Assign temp IDs\nto unsaved elements',        'step','Auto-incremented local IDs'],
        ['guard',   'Set up auto-restore\nfor original IDs',       'step',   'IDs restored on scope exit even if exception'],
        ['elms', 'Write all elements:\ntype tag + element data', 'key', '', 'ge_save'],
        ['conns',   'Write all connections:\ntype tag + wire data', 'key', ''],
        ['done',    'Done\n(IDs auto-restored)',                    'end',      ''],
      ],
  edges: [
        ['start', 'ids'],
        ['ids',   'guard'],
        ['guard', 'elms'],
        ['elms',  'conns'],
        ['conns', 'done'],
      ]
};

flowRegistry['ser_deserialize'] = {
  title: 'Deserialize (Read Items)',
  nodes: [
        ['start',   'Deserialize items\nfrom stream',              'start',    ''],
        ['loop',    'While stream\nhas data',                       'key',      ''],
        ['read_t',  'Read type tag',                                'step',     ''],
        ['d_status','Stream\nvalid?',                               'decision', ''],
        ['ex_s',    'Error:\n"Stream error"',                      'error',    ''],
        ['d_type',  'Item type?',                                   'decision', ''],
        ['elm', 'Element:\nread type \u2192 build \u2192 load', 'key', '', 'ge_load'],
        ['conn', 'Connection:\ncreate \u2192 load', 'key', '', 'ge_load'],
        ['ex_type', 'Error:\n"Invalid type tag"',                  'error',    ''],
        ['append',  'Add to item list',                             'step',     ''],
        ['done',    'Return item list',                             'end',      ''],
      ],
  edges: [
        ['start',   'loop'],
        ['loop',    'read_t'],
        ['read_t',  'd_status'],
        ['d_status','ex_s',      'No'],
        ['d_status','d_type',    'Yes'],
        ['d_type',  'elm',       'Element'],
        ['d_type',  'conn',      'Connection'],
        ['d_type',  'ex_type',   'unknown'],
        ['elm',     'append'],
        ['conn',    'append'],
        ['append',  'loop'],
        ['loop',    'done'],
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
    ['f0', 'Save Flow', 'key', '', 'io_save'],
    ['f1', 'Load Flow', 'key', '', 'io_load'],
    ['f2', 'Serialization Context', 'key', '', 'io_context']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2']
  ]
};

flowRegistry['io_save'] = {
  title: 'Save Flow',
  nodes: [
        ['start',    'Workspace save',                           'start',    ''],
        ['header', 'Write file header', 'step', 'Magic "WPCF" + version number', 'ser_header'],
        ['dolphin',  'Write waveform filename\n+ viewport rect', 'step',     ''],
        ['metadata', 'Write metadata (V4.5+)',                    'step',     'Includes embedded IC blob registry'],
        ['ser', 'Serialize all items', 'key', 'For each item: write type tag \u2192 element or connection data', 'ser_serialize'],
      ],
  edges: [
        ['start',    'header'],
        ['header',   'dolphin'],
        ['dolphin',  'metadata'],
        ['metadata', 'ser'],
      ]
};

flowRegistry['io_load'] = {
  title: 'Load Flow',
  nodes: [
        ['start',    'Workspace load',                           'start',    ''],
        ['header', 'Read file header', 'step', 'Detect format: magic header (modern) vs string (legacy) vs headerless (V4.1)', 'ser_header'],
        ['preamble', 'Read preamble:\nheader + waveform +\nrect + metadata', 'step', ''],
        ['blobs',    'Extract embedded IC\nblobs from metadata', 'step',     ''],
        ['deser', 'Deserialize all items', 'key', 'Loop: read type tag \u2192 build element \u2192 load. Or load connection \u2192 reconnect via port map.', 'ser_deserialize'],
        ['add',      'Add items to scene',                       'end',     'Register IDs, restore connections, trigger simulation rebuild'],
      ],
  edges: [
        ['start',    'header'],
        ['header',   'preamble'],
        ['preamble', 'blobs'],
        ['blobs',    'deser'],
        ['deser',    'add'],
      ]
};

flowRegistry['io_context'] = {
  title: 'Serialization Context',
  nodes: [
        ['ctx',      'Per-load state',                           'key',     'Port map: serial ID \u2192 port. Version: file format version. Working directory. Blob registry: embedded IC blobs.'],
      ],
  edges: []
};
