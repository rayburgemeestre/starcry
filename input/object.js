_ = {
  'gradients': {
    'white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': .99, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'textures': {
    'clouds1': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 7,
      'persistence': 0.45,
      'percentage': 1.0,
      'scale': 2000.,
      'range': [0.0, 0.0, 0.2, 0.8],
      // 'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'zernike1':
        {'type': 'zernike', 'n': 7, 'm': -3, 'zernike_type': zernike_type.version1, 'effect': texture_effect.opacity}
  },
  'objects': {
    'main': {
      'init': function() {
        this.spawn({'id': 'main1'});
        this.spawn({'id': 'main2'});
      },
      'time': function(t, e, s, T) {
        // how do we tween between objects...
      }
    },
    'main1': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      //'radiussize': 100.0,
      'radiussize': 20.0,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, scene) {
        this.opacity = 1.0 - t;
      },
      'gradient': 'white',
      'texture': 'clouds1',
    },
    'main2': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      //'radiussize': 100.0,
      'radiussize': 20.0,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, scene) {
        this.opacity = t;
      },
      'gradient': 'white',
      //'texture': 'clouds1',
      'texture': 'zernike1',
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 5.0,
    'rand_seed': 5,
    'granularity': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': false,
    'gamma': 1.0,
  },
  'preview': {
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {'name': 'scene', 'duration': 5.0, 'objects': [{'id': 'main', 'x': 0, 'y': 0, 'z': 0}]},
  ]
};
