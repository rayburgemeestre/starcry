_ = {
  'gradients': {
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      'width': 1920,
      'height': 1080,
    }
  },
  'objects': {},
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
    'bg_color': {'r': 1., 'g': 1., 'b': 1., 'a': 1},
    'gravity_constrain_dist_min': 50.,
    'gravity_constrain_dist_max': 60.,
    'min_intermediates': 10,
    'max_intermediates': 10,
  },
  'preview': {
    'motion_blur': false,
    'min_intermediates': 2,
    'max_intermediates': 2,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 1920,
    'height': 1080,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 60,
    'objects': [
      {'id': 'ball', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};