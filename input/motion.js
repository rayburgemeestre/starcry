_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
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
      'props': {'subobj': []},
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        let n = 10;
        for (let i = 0; i < n; i++)
          this.props.subobj.push(this.spawn({
            'id': 'ball',
            // 'x': ((rand() * 2.) - 1.) * 1920/2,
            // 'y': ((rand() * 2.) - 1.) * 1920/2,
            // 'x': 0,
            // 'y': 0,
            'z': 0,
            'velocity': rand() * 100.,
            'vel_x': ((rand() * 2.) - 1.),
            'vel_y': ((rand() * 2.) - 1.),
          }));
        for (let i = 0; i < n; i++) {
          for (let j = 0; j < n; j++) {
            if (i <= j) {
              let obj1 = this.props.subobj[i];
              let obj2 = this.props.subobj[j];
              this.props.subobj.push(this.spawn3(
                  {
                    'id': 'line',
                    'opacity': rand(),
                    'z': 0,
                  },
                  obj1,
                  obj2));
            }
          }
        }
      },
      'time': function(t) {
        this.angle = 360. * t;
      },
    },
    'ball': {
      'type': 'circle',
      'opacity': 0,
      // 'collision_group': 'cg1',
      //      'toroidal': 't1',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 10,
      'radiussize': 10.0,
      'props': {'subobj': []},
      'init': function() {
        this.props.seed = rand();
        this.props.vel = this.velocity;
      },
      'time': function(t) {
        this.velocity = this.props.vel * Math.sin(t * 100 * this.props.seed);
      },
    },
    'line': {
      'type': 'line',
      // 'collision_group': 'cg1',
      // 'toroidal': 't1',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 2,
      'init': function() {},
      'time': function(t) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 3,
    'granularity': 1,
    'grain_for_opacity': true,
    'dithering': true,
    'min_intermediates': 30,
    'max_intermediates': 30,
    //'max_intermediates': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'motion_blur': false,
    'min_intermediates': 1,
    'max_intermediates': 1,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 1920,
    'height': 1080,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 10,
    'objects': [
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 3., 'scale': 1., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 3., 'scale': 1., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 3., 'scale': 1., 'props': {}},

      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 6., 'scale': 2., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 7., 'scale': 3., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 8., 'scale': 4., 'props': {}},
    ],
  }]
};
