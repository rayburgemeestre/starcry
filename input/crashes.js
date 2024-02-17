_ = {
  'gradients': {
    'white': '#000000',
    'red': '#ff0000',
  },
  'objects': {
    'balls': {
      'init': function() {
        function radians(degrees) {
          return degrees * (Math.PI / 180);
        }
        let ids = [];
        for (let n = 12, i = 0; i < n; i++) {
          let t = (360 / n) * i;
          ids.push(this.spawn({
            'id': 'ball',
            'x': 100 * Math.cos(radians(t)),
            'y': 100 * Math.sin(radians(t)),
            'z': 0,
            'radiussize': 16,
            // 'mass': 100,
            //   'velocity': 1,
            //    'vel_x': 1 * Math.cos(radians(t + 90)),
            //    'vel_y': 1 * Math.sin(radians(t + 90)),
          }));
        }
        for (let i of ids) {
          for (let j of ids) {
            this.spawn3({'id': 'line'}, i, j);
          }
        }
      },
      'time': function(t, e, s, tt) {
        // this.rotate += e * 20;
      }
    },
    'ball': {
      'type': 'circle',
      'gravity_group': 'group1',
      'opacity': 1.,
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 10.0,
      'collide': function(other) {},
      'init': function() {
        if (false && this.level < 2) {
          let id = this.spawn({
            'id': 'balls',
            'x': 0,
            'y': 0,
            'z': 0,
          });
          // this.spawn3(
          //     {
          //       'id': 'line',
          //     },
          //     this.unique_id,
          //     id);
        }
        this.spawn({'id': 'circle', 'x': 0, 'y': 0});
      }
    },
    'circle': {
      'type': 'circle',
      'gradient': 'red',
      'opacity': 1.,
      'blending_type': blending_type.add,
      'radius': 50,
      'radiussize': 3.0,
    },
    'line': {
      'type': 'line',
      'opacity': 1.,
      'blending_type': blending_type.add,
      'unique_group': 'g',
      'gradient': 'red',
      'radius': 0,
      'radiussize': 1.0,
      'init': function() {},
    },
  },
  'video': {
    // 'fps': 25,
    // 'width': 1920,
    // 'height': 1080,
    'scale': 2,
    // 'rand_seed': 3,
    // 'granularity': 1,
    // 'grain_for_opacity': false,
    // 'dithering': false,
    // 'minimize_steps_per_object': false,
    'min_intermediates': 10,
    'max_intermediates': 10,
    // 'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    // 'gravity_range': 10000,
    // 'gravity_G': 1.0,
    // 'gravity_constrain_dist_min': 50.,
    // 'gravity_constrain_dist_max': 60.,
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
        'id': 'line',
        'x': -100,
        'y': 0,
        'z': 0,
      },
      {
        'id': 'balls',
        'x': 0,
        'y': 0,
        'z': 0,
      },
    ],
  }]
};
