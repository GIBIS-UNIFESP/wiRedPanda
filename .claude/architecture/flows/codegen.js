// Flow definitions: codegen
flowRegistry['arduino_gen_ops'] = {
  title: 'ArduinoCodeGen \u2014 Arduino Export',
  nodes: [
    ['f0', '\u2460 generate()', 'key', '', 'arduino_gen_u2460_generate']
  ],
  edges: [

  ]
};

flowRegistry['arduino_gen_u2460_generate'] = {
  title: '\u2460 generate()',
  nodes: [
        ['start',   'generate()',                             'start',   ''],
        ['open',    'Open output file\nfor writing',         'step',    ''],
        ['header',  'Write #include +\n#define directives',   'step',   ''],
        ['sort',    'Sort elements\nby topology',            'key',     ''],
        ['assign',  'assignVariablesRec()\nfor each element', 'step',   'Map elements to variable names'],
        ['board',   'selectBoard()',                          'step',    ''],
        ['setup',   'Write setup():\ninitialize pins +\ndigitalWrite constants','key',''],
        ['loop',    'Write loop():\ndigitalRead inputs \u2192\ngate logic \u2192\ndigitalWrite outputs','key',''],
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

flowRegistry['sv_gen_ops'] = {
  title: 'SystemVerilogCodeGen \u2014 HDL Export',
  nodes: [
    ['f0', '\u2460 generate()', 'key', '', 'sv_gen_u2460_generate']
  ],
  edges: [

  ]
};

flowRegistry['sv_gen_u2460_generate'] = {
  title: '\u2460 generate()',
  nodes: [
        ['start',   'generate()',                             'start',   ''],
        ['open',    'Open output file',                      'step',    ''],
        ['ic_mod',  'generateICModules()\nfor all IC sub-circuits','key','Recursive: each IC becomes a sub-module'],
        ['mod_hdr', 'Write top-level\nmodule header',        'step',    ''],
        ['inputs',  'Declare input ports',                   'step',    ''],
        ['outputs', 'Declare output ports',                  'step',    ''],
        ['aux',     'Declare aux variables\n(internal wires)','step',   ''],
        ['sort',    'Sort by topology +\nassignVariablesRec()','key',   ''],
        ['comb',    'Write combinational:\nassign statements', 'step',  'AND \u2192 &, OR \u2192 |, NOT \u2192 ~, etc.'],
        ['seq',     'Write sequential:\nalways_ff blocks',    'step',   'Flip-flops + latches'],
        ['fb',      'Write feedback:\nOR gate instantiations','step',   ''],
        ['end_mod', 'endmodule',                              'end',    ''],
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

flowRegistry['codegen_mod'] = {
  title: 'Code Generation',
  nodes: [
    ['f0', 'Export Pipeline', 'key', '', 'codegen_export_pipeline']
  ],
  edges: [

  ]
};

flowRegistry['codegen_export_pipeline'] = {
  title: 'Export Pipeline',
  nodes: [
        ['start',    'MainWindow \u2192\nExport menu',          'start',    ''],
        ['arduino',  'ArduinoCodeGen',                           'step',     'Reads Scene elements in topological order. Maps each to Arduino digital I/O. Generates setup() + loop().'],
        ['verilog',  'SystemVerilogCodeGen',                     'step',     'Reads Scene elements + connections. Maps to module with wire/reg. IC sub-circuits become sub-module instantiations.'],
        ['collect',  '1. Collect elements\nfrom Scene',          'step',     'scene\u2192elements() + sortByTopology()'],
        ['wireless', '2. Resolve wireless\nchannels',            'step',     'wirelessTxInputPorts() maps Rx labels to Tx port signals'],
        ['map',      '3. Map element types to\ntarget language', 'key',     'AND \u2192 & operator, DFlipFlop \u2192 sequential block, IC \u2192 sub-module'],
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

