_ = {
  'gradients': {
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
  },
  'objects': {
    'top': {
      'type': 'script',
      'file': 'input/orbit_b.js',
      'gravity_group': 'group2',
      'duration': 5.0,
      'radiussize': 10.0,
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 3,
    'granularity': 1,
    'grain_for_opacity': false,
    'dithering': false,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'gravity_range': 10000,
    'gravity_G': 1.0,
    'gravity_constrain_dist_min': 5.,
    'gravity_constrain_dist_max': 25.,
  },
  'preview': {
    'motion_blur': false,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 500,
    'height': 500,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 60,
    'objects': [
      {
        'id': 'top',
        'x': 500,
        'y': 0,
        'z': 0,
        'radius': 0,
        'radiussize': 10,
        'mass': 20 * 10.,
        'velocity': 10.,
        'vel_x': -0.1,
        'vel_y': 0,
      },
      {
        'id': 'top',
        'x': -500,
        'y': -80,
        'z': 0,
        'radius': 0,
        'radiussize': 8,
        'mass': 20.,
        'velocity': 1. * 10.,
        'vel_x': 1.,
        'vel_y': 0,
      },
    ],
  }]
};