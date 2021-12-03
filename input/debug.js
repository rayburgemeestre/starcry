_ = {
  'gradients': {
    'white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.1, 'r': 1, 'g': 1, 'b': 1, 'a': 0.5},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'circle': {
      'type': 'circle',
      'gradient': 'white',
      'blending_type': blending_type.phoenix,
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 50,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, scene) {
        //        this.angle = t * 360;
        // this.x = rand();
        // this.y = rand();
      },
    },
  },
  'video': {
    'fps': 30,
    'width': 1080,
    'height': 1080,
    'scale': 2,
    'rand_seed': 5,
    'granularity': 1,
    // 'min_intermediates': 5.,
    // 'max_intermediates': 50.,
    'max_intermediates': 1.,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 0},
    'grain_for_opacity': false,
  },
  'preview': {},
  'scenes': [
    {
      'name': 'scene1',
      'duration': 5,
      'objects': [
        {'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': 0, 'y': 5, 'z': 0, 'props': {}},
      ]
    },
  ]
};
