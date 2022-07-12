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
      'id': 'balls_tpl',
      'x': 0,
      'y': 0,
      'init': function() {
        for (let i = 0; i < 10000; i++) {
          this.spawn({
            'id': 'ball',
            'x': rand() * 1000 - 500,
            'y': rand() * 1000 - 500,
            'z': 0,
            'velocity': 10.,
            'vel_x': Math.random(),
            'vel_y': Math.random(),
          });
        }
      }
    },
    'ball': {
      'id': 'ball_tpl',
      'type': 'circle',
      //'collision_group': 'group1',
      //'gravity_group': 'group1',
      //'toroidal': 't1',
      'blending_type': blending_type.normal,
      'gradient': 'blue',
      'radius': 0,
      'radiussize': 5.0,
      'init': function() {
        //  output("in init function for object id ball");
      },
      'time': function(t, e, s, tt) {
        // output("in time function for object id ball with: " + t + " / " + e + " / " + s + " / " + tt);
        // this.x += 1;
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
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
