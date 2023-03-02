_ = {
  'gradients': {},
  'objects': {
    'orbitz': {
      'type': 'script',
      'file': 'input/orbit_b_b.js',
    },
    'text': {
      'type': 'script',
      'file': 'input/text.js',
    },
    'split': {
      'type': 'script',
      'file': 'input/split.js',
      'duration': 5.0,
    },
    'motion': {
      'type': 'script',
      'file': 'input/subobj.js',
    },
    // 'greenlines': {
    //   'type': 'script',
    //   'file': 'input/greenlines.js',
    //   'time': function(t, e, s, tt) {
    //     this.scale = 1.0 + 0.1 * Math.sin(t * 2 * Math.PI);
    //   }
    // },
    //'memleak': {
    //  'type': 'script',
    //  'file': 'input/memory_leak.js',
    //},
    'subscript': {
      'type': 'script',
      'file': 'input/subscript.js',
    },
    'orbit': {
      'type': 'script',
      'file': 'input/orbit2.js',
    },
    'logo': {
      'type': 'script',
      'file': 'input/logo.js',
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.0,  // TODO: bug if you decrease this to 0.5!
    'rand_seed': 5,
    'granularity': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': true,
    'min_intermediates': 10,
    'max_intermediates': 10,
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
      'duration': 5.0,
      'objects': [
        {'id': 'split', 'label': 'split', 'x': 0, 'y': -100, 'z': 0},
        // {'id': 'memleak', 'label': 'memleak', 'x': 0, 'y': 0, 'z': 0},
        {'id': 'motion', 'label': 'motion', 'x': 0, 'y': 0, 'z': 0},
        {'id': 'orbit', 'label': 'orbit1', 'x': -500, 'y': 0, 'z': 0},
        {'id': 'orbit', 'label': 'orbit2', 'x': 500, 'y': 0, 'z': 0},
        {'id': 'logo', 'label': 'logo', 'x': 0, 'y': 100, 'z': 0},
        {'id': 'orbitz', 'label': 'orbitz', 'x': 0, 'y': -100, 'z': 0},
        // {'id': 'greenlines', 'label': 'g1', 'x': -100, 'y': -100, 'z': 0},
      ],
    },
    {
      'name': 'scene2',
      'duration': 5.0,
      'objects': [
        {'id': 'split', 'label': 'split', 'x': 0, 'y': -100, 'z': 0},
        {'id': 'text', 'label': 'text', 'x': 0, 'y': 0, 'z': 0},
        {'id': 'subscript', 'label': 'subscript', 'x': 0, 'y': 0, 'z': 0, 'scale': 0.5},
      ]
    },
  ]
};
