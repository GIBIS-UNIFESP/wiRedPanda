// Flow definitions: codegen
flowRegistry['cg_arduino'] = {
  title: 'Arduino Code Generation',
  nodes: [
        ['start',   'Generate Arduino code',                 'start',   ''],
        ['open',    'Open output file\nfor writing',         'step',    ''],
        ['header',  'Write #include and\n#define directives', 'step',   ''],
        ['sort',    'Sort elements\nby topology',            'key',     ''],
        ['assign',  'Assign variable names\nto each element', 'step',   ''],
        ['board',   'Select board type',                     'step',    ''],
        ['setup',   'Write setup():\ninitialize pins +\nwrite constants','key',''],
        ['loop',    'Write loop():\nread inputs \u2192\ngate logic \u2192\nwrite outputs','key',''],
        ['helpers', 'Write helper functions\nfor each gate type','step',''],
        ['close',   'Close file',                             'end',    ''],
      ],
  edges: [
        ['start',   'open'],
        ['open',    'header'],
        ['header',  'sort'],
        ['sort',    'assign'],
        ['assign',  'board'],
        ['board',   'setup'],
        ['setup',   'loop'],
        ['loop',    'helpers'],
        ['helpers', 'close'],
      ]
};

flowRegistry['cg_verilog'] = {
  title: 'SystemVerilog Code Generation',
  nodes: [
        ['start',   'Generate SystemVerilog',                'start',   ''],
        ['open',    'Open output file',                      'step',    ''],
        ['ic_mod',  'Generate IC sub-modules\n(recursive)',  'key','Each IC becomes a sub-module'],
        ['mod_hdr', 'Write top-level\nmodule header',        'step',    ''],
        ['inputs',  'Declare input ports',                   'step',    ''],
        ['outputs', 'Declare output ports',                  'step',    ''],
        ['aux',     'Declare auxiliary\nvariables (internal wires)',  'step',   ''],
        ['sort',    'Sort by topology,\nassign variable names','key',   ''],
        ['comb',    'Write combinational:\nassign statements', 'step',  'AND \u2192 &, OR \u2192 |, NOT \u2192 ~, etc.'],
        ['seq',     'Write sequential:\nalways_ff blocks',    'step',   'Flip-flops + latches'],
        ['fb',      'Write feedback:\nOR gate instantiations','step',   ''],
        ['end_mod', 'Close module',                           'end',    ''],
      ],
  edges: [
        ['start',   'open'],
        ['open',    'ic_mod'],
        ['ic_mod',  'mod_hdr'],
        ['mod_hdr', 'inputs'],
        ['inputs',  'outputs'],
        ['outputs', 'aux'],
        ['aux',     'sort'],
        ['sort',    'comb'],
        ['comb',    'seq'],
        ['seq',     'fb'],
        ['fb',      'end_mod'],
      ]
};

flowRegistry['cg_pipeline'] = {
  title: 'Export Pipeline',
  nodes: [
        ['start',    'MainWindow \u2192\nExport menu',          'start',    ''],
        ['arduino',  'Arduino Code Generator',                   'step',     'Maps circuit to Arduino digital I/O. Generates setup() + loop().'],
        ['verilog',  'SystemVerilog Generator',                  'step',     'Maps circuit to module with wire/reg. IC sub-circuits become sub-module instantiations.'],
        ['collect',  '1. Collect elements\nfrom scene',          'step',     'Sort by topology'],
        ['wireless', '2. Resolve wireless\nchannels',            'step',     'Map Rx labels to Tx port signals'],
        ['map',      '3. Map element types\nto target language', 'key',     'AND \u2192 & operator, DFlipFlop \u2192 sequential block, IC \u2192 sub-module'],
        ['write',    '4. Write to\nfile / clipboard',            'end',      ''],
      ],
  edges: [
        ['start',    'arduino'],
        ['start',    'verilog'],
        ['arduino',  'collect'],
        ['verilog',  'collect'],
        ['collect',  'wireless'],
        ['wireless', 'map'],
        ['map',      'write'],
      ]
};
