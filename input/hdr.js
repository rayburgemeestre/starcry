_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.1, 'r': 0, 'g': 1, 'b': 0, 'a': 0.1},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.1, 'r': 0, 'g': 0, 'b': 1, 'a': 0.1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
  },
  'textures': {
    'clouds': {
      'type': 'turbulence',
      'size': 512.,
      'octaves': 6,
      'persistence': 0.7,
      'percentage': 1.,
      'scale': 50.,
      'range': [0.0, 0.1, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
  },
  'objects': {
    'camera': {
      'init': function() {},
      'time': function(t, e, s, gt) {
        // script.video.scale = 10 - 9.0 * logn(gt, 1000);
      },
    },
    'background': {
      'type': 'circle',
      'gradient': 'red',
      'texture': 'clouds',
      'radius': 0.0,
      'radiussize': 600.0,
      'blending_type': blending_type.normal,
    },
    'mother': {
      'x': 0,
      'y': 0,
      'angle': 0,
      'init': function() {
        this.spawn({
          'id': 'green_circle',
          'x': 0,
          'y': 0,
          'pivot': true,
          'scale': 1.0,
          'props': {'radius_limit': 15., 'opacity': 1.0, 'parent': false, 'scale': 1.0, 'parent_radius': 100}
        });
      },
      'time': function(t, e, s, gt) {
        // angle is towards parent
        // we need also something like 'rotation' here..
        // this.angle -= e * 20;
      }
    },
    'green_circle': {
      'type': 'circle',
      'gradient': 'green',
      // 'gradients': [
      //   [1.0, 'blue'],
      //   [0.0, 'green'],
      // ],
      // temporary commentedfor testing purposes
      'texture': 'clouds',
      'radius': 100.,
      'radiussize': 30.0,
      'opacity': 1.,
      'blending_type': blending_type.pinlight,
      'angle': 0,  // script.video.mode === 1 ? 1 : 0,
      'props': {
        'spawned': 0,
      },
      'scale': 1.0,
      'init': function() {},
      'time': function(time, elapsed, scene, global_time) {
        if (this.radius > this.props.radius_limit && this.props.spawned === 0) {
          var child_radius = this.radius * 0.67;
          this.spawn({
            'id': 'green_circle',
            // keep somehow the parent radius...
            // anyway going to render a nice vid anyway
            'x': 0,  // this.radius - child_radius,
            'y': 0,
            'radius': child_radius,
            'props': {
              'parent_radius': this.props.parent_radius,
              'radius_limit': this.props.radius_limit,
              'scale': 1.0,
            },
          });
          this.props.spawned++;
        }
        // pivot = false
        // this.x = -1 * ((this.radius / 0.67) - this.radius);
        // pivot = true
        this.x = -1 * (this.props.parent_radius - this.radius);
        this.radius += elapsed * 100;
        this.props.parent_radius += elapsed * 100;
        // this.angle -= elapsed * 100 * (this.level / 1.);
        this.angle -= elapsed * 100 + (this.level * 10.);
      },
    },
  },
  'video': {
    'mode': 1,
    'fps': 25,
    //'width': 1920 * 2.,
    //'height': 1920 * 2.,
    //'scale': 10. * 2.,
    'width': 1920,
    'height': 1920,
    'scale': 1.0,
    'granularity': 1,
    'grain_for_opacity': true,
    'motion_blur': true,
    // 'min_intermediates': 30.,
    // 'max_intermediates': 100.,
    'minimize_steps_per_object': false,
    'perlin_noise': true,
    'dithering': true,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    // 'width': 320,
    // 'height': 320,
    'granularity': 1.0,
    //'motion_blur': false,
    //'max_intermediates': 1.,
    'perlin_noise': false,
    'dithering': false,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 10,
      'objects': [
        {'id': 'camera', 'x': 0, 'y': 0},
        {'id': 'mother', 'x': 0, 'y': 0},
      ],
    },
  ]
};
