// Flow definitions: serialization
flowRegistry['serialization_ops'] = {
  title: 'Serialization \u2014 File Format Engine',
  nodes: [
    ['f0', '\u2460 serialize()', 'key', '', 'serialization_u2460_serialize'],
    ['f1', '\u2461 deserialize()', 'key', '', 'serialization_u2461_deserialize'],
    ['f2', '\u2462 readPandaHeader', 'key', '', 'serialization_u2462_readpandaheader']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2']
  ]
};

flowRegistry['serialization_u2460_serialize'] = {
  title: '\u2460 serialize()',
  nodes: [
        ['start',   'serialize(items, stream)',              'start',    ''],
        ['ids',     'Assign temp IDs to\nunsaved elements (id \u2264 0)','step','localId = max existing ID; increment per element'],
        ['guard',   'qScopeGuard:\nauto-restore original IDs','step',   'IDs restored on scope exit even if exception'],
        ['elms',    'Write all GraphicElements:\ntype tag + elementType\n+ element.save()', 'key', ''],
        ['conns',   'Write all QNEConnections:\ntype tag + conn.save()','key', ''],
        ['done',    'return',                                'end',      'Scope guard restores temp IDs'],
      ],
  edges: [
        ['start', 'ids'],
        ['ids',   'guard'],
        ['guard', 'elms'],
        ['elms',  'conns'],
        ['conns', 'done'],
      ]
};

flowRegistry['serialization_u2461_deserialize'] = {
  title: '\u2461 deserialize()',
  nodes: [
        ['start',   'deserialize(stream, context)',          'start',    ''],
        ['loop',    'while !stream.atEnd()',                  'key',      ''],
        ['read_t',  'stream >> type tag',                    'step',     ''],
        ['d_status','stream.status()\n== Ok?',               'decision', ''],
        ['ex_s',    'throw PANDACEPTION\n"stream error"',    'error',    ''],
        ['d_type',  'type?',                                 'decision', ''],
        ['elm',     'GraphicElement:\nread elmType \u2192\nbuildElement \u2192\nelm.load(stream, ctx)', 'key', ''],
        ['conn',    'QNEConnection:\nnew conn \u2192\nconn.load(stream, ctx)', 'key', ''],
        ['ex_type', 'throw PANDACEPTION\n"Invalid type"',    'error',    'Unknown type tag in stream'],
        ['append',  'itemList.append(item)',                  'step',     ''],
        ['done',    'return itemList',                        'end',      ''],
      ],
  edges: [
        ['start',   'loop'],
        ['loop',    'read_t'],
        ['read_t',  'd_status'],
        ['d_status','ex_s',      'No'],
        ['d_status','d_type',    'Yes'],
        ['d_type',  'elm',       'GraphicElement'],
        ['d_type',  'conn',      'QNEConnection'],
        ['d_type',  'ex_type',   'unknown'],
        ['elm',     'append'],
        ['conn',    'append'],
        ['append',  'loop'],
        ['loop',    'done'],
      ]
};

flowRegistry['serialization_u2462_readpandaheader'] = {
  title: '\u2462 readPandaHeader',
  nodes: [
        ['start',   'readPandaHeader(stream)',               'start',    'Detects file format: modern, legacy string, or headerless'],
        ['ver_qt',  'stream.setVersion(Qt_5_12)',            'step',     ''],
        ['save_p',  'Save stream position',                  'step',     ''],
        ['read_m',  'Read 4 bytes as quint32',               'step',     ''],
        ['d_magic', 'Matches\nMAGIC_HEADER?',                'decision', '0x57504346 ("WPCF")'],
        ['mod_ver', 'Read QVersionNumber\n\u2192 version',   'end',      'Modern format (V4.5+)'],
        ['rewind1', 'Seek back to saved pos',                'step',     ''],
        ['read_s',  'Read QString appName',                  'step',     ''],
        ['d_empty', 'appName\nempty?',                       'decision', ''],
        ['rewind2', 'Seek back to saved pos',                'step',     ''],
        ['read_pt', 'Read QPointF center',                   'step',     ''],
        ['d_null',  'center\nisNull?',                       'decision', ''],
        ['ex_inv',  'throw PANDACEPTION\n"Invalid file format"','error', ''],
        ['rewind3', 'Seek back to saved pos',                'step',     ''],
        ['v41',     'version = 4.1\n(last headerless release)','end',    ''],
        ['d_wp',    'Starts with\n"wiRedPanda"?',            'decision', ''],
        ['parse',   'Parse version from\n"wiRedPanda X.Y" string','end', 'Legacy string header (V4.2\u20134.4)'],
        ['ex_inv2', 'throw PANDACEPTION\n"Invalid file format"','error', ''],
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
    ['f0', 'Save Flow', 'key', '', 'io_save_flow'],
    ['f1', 'Load Flow', 'key', '', 'io_load_flow'],
    ['f2', 'SerializationContext', 'key', '', 'io_serializationcontext']
  ],
  edges: [
    ['f0', 'f1'],
    ['f1', 'f2']
  ]
};

flowRegistry['io_save_flow'] = {
  title: 'Save Flow',
  nodes: [
        ['start',    'Workspace::save(fileName)',                'start',    ''],
        ['header',   'writePandaHeader(stream)',                 'step',     'Magic: 0x57504346 ("WPCF") + version number'],
        ['dolphin',  'Write dolphin filename\n+ viewport rect', 'step',     ''],
        ['metadata', 'Write metadata (V4.5+)',                   'step',     'Includes embedded IC blob registry'],
        ['ser',      'Serialization::serialize\n(items, stream)', 'key',    'For each item: write type tag \u2192 element.save() or connection.save()'],
      ],
  edges: [
        ['start',    'header'],
        ['header',   'dolphin'],
        ['dolphin',  'metadata'],
        ['metadata', 'ser'],
      ]
};

flowRegistry['io_load_flow'] = {
  title: 'Load Flow',
  nodes: [
        ['start',    'Workspace::load(fileName)',                'start',    ''],
        ['header',   'readPandaHeader(stream)',                  'step',     'Detect format: magic header (modern) vs "wiRedPanda X.Y" string (legacy) vs headerless (V4.1)'],
        ['preamble', 'readPreamble()',                           'step',     'Header + dolphin filename + rect + metadata'],
        ['blobs',    'deserializeBlobRegistry()',                 'step',     'Extract embedded IC blobs from metadata'],
        ['deser',    'Serialization::deserialize\n(stream, context)', 'key', 'Loop: read type tag \u2192 buildElement(type) \u2192 element.load(stream, ctx). Or QNEConnection.load() \u2192 reconnect via portMap.'],
        ['add',      'Add items to Scene',                       'end',     'Register IDs, restore connections, trigger simulation rebuild'],
      ],
  edges: [
        ['start',    'header'],
        ['header',   'preamble'],
        ['preamble', 'blobs'],
        ['blobs',    'deser'],
        ['deser',    'add'],
      ]
};

flowRegistry['io_serializationcontext'] = {
  title: 'SerializationContext',
  nodes: [
        ['ctx',      'Per-load state',                           'key',     'portMap: serial ID \u2192 QNEPort*. version: file format version. contextDir: .panda file directory. blobRegistry: embedded IC blobs.'],
      ],
  edges: []
};

