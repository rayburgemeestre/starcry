_ = {
  'gradients': {
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      'width': 1920,
      'height': 1080,
    }
  },
  'objects': {
    'balls': {
      'x': 0,
      'y': 0,
      'init': function() {
        let step = 100;
        for (let i = 0; i < 5; i++) {
          for (let j = 0; j < 5; j++) {
            let mass = Math.random() * 20;
            this.subobj.push(this.spawn({
              'id': 'ball',
              'x': i * step - (2.5 * step),
              'y': j * step - (2.5 * step),
              'z': 0,
              'velocity': 10.,
              'radiussize': 10. + mass,
              'mass': 1. + mass,
              'vel_x': Math.random() * 2. - 1.,
              'vel_y': Math.random() * 2. - 1.,
            }));
          }
        }
      },
    },
    'ball': {
      'type': 'circle',
      'collision_group': 'group1',
      // 'toroidal': 't1', --- toroidal seems broken
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
    'bg_color': {'r': 1., 'g': 1., 'b': 1., 'a': 1},
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
    'duration': 5,
    'objects': [
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
