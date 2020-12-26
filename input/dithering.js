_ = {
  'gradients': {
    'green': [
      {'position': 0.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0.3},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0.0},
    ],
  },
  'textures': {},
  'objects': {
    'bg': {
      'type': 'circle',
      'gradient': 'green',
      'radius': 0,
      'radiussize': 1920,
      'opacity': 1.0,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'duration': 2,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.,
    'rand_seed': 1,
    'granularity': 2,
    'extra_grain': 0.1,
    'grain_for_opacity': true,
    'dithering': true,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      //{'id': 'bg', 'x': -1920/2., 'y': 0, 'z': 0, 'props': {}},
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
