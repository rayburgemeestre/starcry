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
  'objects': {
    'balls': {
      'x': 0,
      'y': 0,
      'init': function() {
        let step = 100;
        for (let i = 0; i < 5; i++) {
          for (let j = 0; j < 5; j++) {
            let mass = rand() * 20;
            this.spawn({
              'id': 'ball',
              'x': i * step - (2.5 * step),
              'y': j * step - (2.5 * step),
              'z': 0,
              'velocity': 10.,
              'radiussize': 10. + mass,
              'mass': 1. + mass,
              'vel_x': rand() * 2. - 1.,
              'vel_y': rand() * 2. - 1.,
            });
          }
        }
      },
    },
    'ball': {
      'type': 'circle',
      'collision_group': 'group1',
      'gravity_group': 'group1',
      'toroidal': 't1',
      'blending_type': blending_type.normal,
      'gradients': [[1, 'blue'], [0, 'red']],
      'radius': 0,
      'radiussize': 10.0,
      'on': {
        'collide': function(other) {
          this.gradients[0][0] = 0.0;
          this.gradients[1][0] = 1.0;
          if (this.gradients[0][0] < 0.0) this.gradients[0][0] = 0.;
          if (this.gradients[1][0] > 1.0) this.gradients[1][0] = 1.;
        }
      },
      'time': function(t, e) {
        if (this.gradients[0][0] < 1.0) {
          this.gradients[0][0] += e;
          this.gradients[1][0] -= e;
        }
        if (this.gradients[0][0] > 1.0) this.gradients[0][0] = 1.;
        if (this.gradients[1][0] < 0.0) this.gradients[1][0] = 0.;
      }
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
    'gravity_constrain_dist_min': 50.,
    'gravity_constrain_dist_max': 60.,
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
    'duration': 10,
    'objects': [
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
