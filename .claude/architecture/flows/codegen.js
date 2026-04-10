// Flow definitions: codegen
flowRegistry['cg_arduino'] = {
  title: 'Arduino Code Generation',
  nodes: [
        ['start',   'Generate Arduino code',                 'start',   ''],
        ['open',    'Open output file\nfor writing',         'step',    ''],
        ['header',  'Write #include and\n#define directives', 'step',   ''],
        ['sort',    'Sort elements\nby topology',            'key',     ''],
        ['board',   'Select board type',                     'step',    'Determines available pins'],
        ['assign',  'Assign variable names\nto each element', 'step',   ''],
        ['setup',   'Write setup():\ninitialize pins +\nwrite constants','key',''],
        ['loop',    'Write loop():\nread inputs \u2192\nupdate clocks \u2192\ngate logic \u2192\nwrite outputs','key',''],
        ['helpers', 'Write helper functions\nfor each gate type','step',''],
        ['close',   'Close file',                             'end',    ''],
      ],
  edges: [
        ['start',   'open'],
        ['open',    'header'],
        ['header',  'sort'],
        ['sort',    'board'],
        ['board',   'assign'],
        ['assign',  'setup'],
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
        ['sort',    'Sort by topology',                        'key',   ''],
        ['assign',  'For each element:\nassign variables and\nemit code in one pass', 'key', 'Logic gates \u2192 assign, flip-flops \u2192 always_ff, latches \u2192 always_latch'],
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
        ['sort',    'assign'],
        ['assign',  'end_mod'],
      ]
};

