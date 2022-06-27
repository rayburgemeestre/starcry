_ = {
  'gradients': {
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
  },
  'objects': {
    'ball': {
      'type': 'circle',
      'gravity_group': 'group1',
      'blending_type': blending_type.normal,
      'gradient': 'blue',
      'radius': 0,
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
    // 'min_intermediates': 1,
    // 'max_intermediates': 1,
    'bg_color': {'r': 1., 'g': 1., 'b': 1., 'a': 1},
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
    'duration': 10,
    'objects': [
      {
        'id': 'ball',
        'x': 0,
        'y': 0,
        'z': 0,
        'radiussize': 20,
        'mass': 20 * 10.,
        'velocity': 0.,
        'vel_x': 0,
        'vel_y': 0,
      },
      {
        'id': 'ball',
        'x': -20,
        'y': -80,
        'z': 0,
        'radiussize': 16,
        'mass': 2.,
        'velocity': 1. * 10.,
        'vel_x': 1.,
        'vel_y': 0,
      },
    ],
  }]
};
