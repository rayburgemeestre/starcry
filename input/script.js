_ = {
  'gradients': {},
  'objects': {
    'split': {
      'type': 'script',
      'file': 'input/split.js',
      'duration': 4.0,
    },
    'motion': {
      'type': 'script',
      'file': 'input/subobj.js',
      'duration': 1.0,
    },
    'subscript': {
      'type': 'script',
      'file': 'input/subscript.js',
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
        {'id': 'split', 'label': 'split', 'x': 0, 'y': -100, 'z': 0},
        {'id': 'motion', 'label': 'motion', 'x': 0, 'y': 0, 'z': 0},
      ],
    },
    {
      'name': 'scene2',
      'duration': 2.0,
      'objects': [
        {'id': 'subscript', 'label': 'subscript', 'x': 0, 'y': 0, 'z': 0, 'scale': 0.5},
      ]
    },
  ]
};
