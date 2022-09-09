_ = {
  'gradients': {
    'white2': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'balls': {
      'init': function() {
        function radians(degrees) {
          var pi = Math.PI;
          return degrees * (pi / 180);
        }

        let n = 6;
        for (let i = 0; i < n; i++) {
          let t = (360 / n) * i;
          this.spawn({
            'id': 'ball',
            'x': 50 * Math.cos(radians(t)),
            'y': 50 * Math.sin(radians(t)),
            'z': 0,
            'radiussize': 16,
            'mass': 1.,
            'velocity': 1,
            'vel_x': 5 * Math.cos(radians(t + 90)),
            'vel_y': 5 * Math.sin(radians(t + 90)),
          });
        }

        n = 8;
        for (let i = 0; i < n; i++) {
          let t = (360 / n) * i;
          this.spawn({
            'id': 'ball',
            'x': 150 * Math.cos(radians(t)),
            'y': 150 * Math.sin(radians(t)),
            'z': 0,
            'radiussize': 16,
            'mass': 1.,
            'velocity': 1,
            'vel_x': 10 * Math.cos(radians(t - 90)),
            'vel_y': 10 * Math.sin(radians(t - 90)),
          });
        }

        n = 32;
        for (let i = 0; i < n; i++) {
          let t = (360 / n) * i;
          this.spawn({
            'id': 'ball',
            'x': 200 * Math.cos(radians(t)),
            'y': 200 * Math.sin(radians(t)),
            'z': 0,
            'radiussize': 6,
            'mass': 0.1,
            'velocity': 1,
            'vel_x': 10 * Math.cos(radians(t + 90)),
            'vel_y': 10 * Math.sin(radians(t + 90)),
          });
        }
      },
    },
    'ball': {
      'type': 'circle',
      'gravity_group': 'group1' + Math.random(),
      'collision_group': 'group1',
      'opacity': 1.,
      'blending_type': blending_type.normal,
      'gradients': [
        [0.5, 'blue'],
        [0.5, 'white'],
      ],
      'radius': 0,
      'radiussize': 10.0,
      'on': {
        'collide': function(other) {
          this.gradients[0][0] = 0.0;
          this.gradients[1][0] = 1.0;
          return;
          let obj = this.spawn({
            'id': 'tempring',
            'label': 'ring_' + this.label,
            'x': 0,
            'y': 0,
            'z': 0,
            'radius': this.radiussize,
            'props': {}
          });
        }
      },
      'time': function(t, e, s, tt) {
        if (this.gradients[0][0] < 1.0) {
          this.gradients[0][0] += e * 1;
          this.gradients[1][0] -= e * 1;
        }
        if (this.gradients[0][0] > 1.0) {
          this.gradients[0][0] = 1.0;
          this.gradients[1][0] = 0.0;
        } else if (s == 1) {
          this.gravity_group = 'group';
        } else if (s == 2) {
          this.opacity = 1. - t;
        }
      }
    },
    'tempring': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 2.0,
      'opacity': 0.5,
      'blending_type': blending_type.add,
      'gradient': 'white',
      'props': {},
      'init': function() {
        this.props.radius = this.radius;
        this.props.elapsed = 1;
      },
      'time': function(t, elapsed) {
        // this.props.elapsed *= 1.1;
        this.props.elapsed = 50;
        this.radius += elapsed * this.props.elapsed;
        if (this.radius >= this.props.radius + 100) {
          this.exists = false;
        }
        this.opacity -= elapsed * 1;
        if (this.opacity < 0) {
          this.opacity = 0;
        }
      },
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
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'gravity_range': 10000,
    'gravity_G': 5.0,
    'gravity_constrain_dist_min': 50.,
    'gravity_constrain_dist_max': 60.,
  },
  'preview': {
    'motion_blur': false,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 500,
    'height': 500,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 8,
      'objects': [
        {
          'id': 'ball',
          'label': 'bossball',
          'gradient': 'white2',
          'x': 0,
          'y': 0,
          'z': 0,
          'radius': 20,
          'radiussize': 5,
          'mass': 20 * 10.,
          'velocity': 0.,
          'vel_x': 0,
          'vel_y': 0,
        },
        {
          'id': 'balls',
          'x': 0,
          'y': 0,
          'z': 0,
        },
        // {
        //   'id': 'balls',
        //   'x': -100,
        //   'y': 0,
        //   'z': 0,
        // },
      ],
    },
    {
      'name': 'scene2',
      'duration': 8,
      'objects': [],
    },
    {
      'name': 'scene3',
      'duration': 4,
      'objects': [],
    }
  ]
};
