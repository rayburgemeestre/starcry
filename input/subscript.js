_ = {
  'gradients': {},
  'objects': {
    'test': {
      'type': 'script',
      'file': 'input/test.js',
      'props': {},
      'init': function() {},
      'time': function(t, e, s, tt) {},
    },
    'blur': {
      'type': 'script',
      'file': 'input/blur.js',
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
        {'id': 'test', 'label': 'split 1', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'blur', 'label': 'split 1', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ],
    },
  ]
};
