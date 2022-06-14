_ = {
  'gradients': {},
  'objects': {
    'split': {
      'type': 'script',
      'file': 'input/split.js',
      'props': {},
      'init': function() {},
      'time': function(t, e, s, tt) {},
    },
    'motion': {
      'type': 'script',
      'file': 'input/subobj.js',
      'props': {},
      'init': function() {},
      'time': function(t, e, s, tt) {},
    },
    'subscript': {
      'type': 'script',
      'file': 'input/subscript.js',
      'props': {},
      'init': function() {},
      'time': function(t, e, s, tt) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.0,
    'rand_seed': 5,
    'granularity': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': false,
  },
  'preview': {
    'width': 512,
    'height': 512,
    'scale': 0.5,
    'max_granularity': 1,
    'max_intermediates': 1,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 2.0,
      'objects': [
        {'id': 'split', 'label': 'split 1', 'x': 0, 'y': -100, 'z': 0, 'props': {}},
        {'id': 'motion', 'label': 'motion', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ],
    },
    {
      'name': 'scene2',
      'duration': 2.0,
      'objects': [
        {'id': 'subscript', 'label': 'split 2', 'x': 0, 'y': 0, 'z': 0, 'props': {}, 'scale': 0.5},
      ]
    },
  ]
};
