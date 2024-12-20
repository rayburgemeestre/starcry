script = {
  'gradients': {},
  'objects': {
    'test': {
      'type': 'script',
      'file': 'input/test.js',
    },
    'subobj': {
      'type': 'script',
      'file': 'input/subobj.js',
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
    'grain_for_opacity': true,
    'minimize_steps_per_object': false,
    'min_intermediates': 10,
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
      'duration': 10.0,
      'objects': [
        {'id': 'test', 'label': 'test', 'x': 0, 'y': 0, 'z': 0},
        {'id': 'test', 'label': 'test2', 'x': 0, 'y': -50, 'z': 0},
        {'id': 'test', 'label': 'test3', 'x': 0, 'y': 50, 'z': 0},
        {'id': 'subobj', 'label': 'subobj', 'x': 0, 'y': 0, 'z': 0},
      ],
    },
  ]
};