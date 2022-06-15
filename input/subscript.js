_ = {
  'gradients': {},
  'objects': {
    'test': {
      'type': 'script',
      'file': 'input/test.js',
      'duration': 1.5,
    },
    'blur': {
      'type': 'script',
      'file': 'input/blur.js',
      'duration': 2.0,
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.0,
    'rand_seed': 5,
    'granularity': 1,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
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
        {'id': 'test', 'x': 0, 'y': 0, 'z': 0},
        {'id': 'blur', 'x': 0, 'y': 0, 'z': 0},
      ],
    },
  ]
};
