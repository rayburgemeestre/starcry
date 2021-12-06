_ = {
  'gradients': {
    'red': [
      {'position': 0, 'r': 1., 'g': 0., 'b': 0., 'a': 1},
      {'position': 1, 'r': 1., 'g': 0., 'b': 0., 'a': 0},
    ],
    'rainbow': [
      {'position': 0 / 7., 'r': 220 / 255., 'g': 29 / 255., 'b': 11 / 255., 'a': 1},
      {'position': 2 / 7., 'r': 251 / 255., 'g': 129 / 255., 'b': 0 / 255., 'a': 1},
      {'position': 3 / 7., 'r': 245 / 255., 'g': 225 / 255., 'b': 12 / 255., 'a': 1},
      {'position': 4 / 7., 'r': 71 / 255., 'g': 213 / 255., 'b': 53 / 255., 'a': 1},
      {'position': 5 / 7., 'r': 43 / 255., 'g': 15 / 255., 'b': 223 / 255., 'a': 1},
      {'position': 6 / 7., 'r': 194 / 255., 'g': 16 / 255., 'b': 169 / 255., 'a': 1},
      {'position': 7 / 7., 'r': 194 / 255., 'g': 16 / 255., 'b': 169 / 255., 'a': 0},
    ],
  },
  'objects': {
    'rainbow': {
      'type': 'circle',
      'gradient': 'rainbow',
      'blending_type': blending_type.normal,
      'collision_group': 'c1',
      'toroidal': 't1',
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 200.0,
      'angle': 0.,
      'subobj': [],
      'init': function() {},
      'time': function(t, e, scene) {},
    },
  },
  'video': {
    'fps': 30,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 5,
    'granularity': 1,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 0},
    'grain_for_opacity': false,
    'dithering': true,
  },
  'preview': {
    'max_intermediates': 1.,
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 30,
      'objects': [
        {'id': 'rainbow', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
  ]
};
