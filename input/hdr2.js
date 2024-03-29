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
        // temporary changed for testing purposes
        if (false)
          this.spawn({
            'id': 'blue_circle',
            //'x': -450,
            'x': -400,
            'y': 0,
            //'scale': 0.5,
            'scale': 1.0,
            'props': {'radius_limit': 60., 'opacity': 1.0}
          });  // 50.
        if (false)
          this.spawn({
            'id': 'blue_circle',
            'x': 400,
            'y': 0,
            'scale': 1.0,
            'props': {'radius_limit': 60., 'opacity': 1.0, 'scale': 1.0}
          });  // 20.
        if (true)
          this.spawn({
            'id': 'blue_circle',
            'x': 0,
            'y': 0,
            'scale': 1.0,
            // 'pivot': true,
            //'props': {'radius_limit': 5., 'opacity': 1.0, 'parent': false, 'scale': 1.0}
            'props': {'radius_limit': 5., 'opacity': 1.0, 'parent': false, 'scale': 1.0, 'px': 0, 'py': 0}
          });  // was 5.
      },
    },
    'blue_circle': {
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
      'opacity': 0.,
      'blending_type': blending_type.pinlight,
      'angle': script.video.mode === 1 ? 1 : 0,
      'props': {
        'parent_radius': false,  // create pivot , set to true, and then all will have this.pivot()
        'extra_radius': 0,
        'scale': 1.0,
        'spawned': 0,
        'px': 0,
        'py': 0,
      },
      'scale': 1.0,
      'init': function() {
        if (this.props.parent_radius === false) {
          this.props.parent_radius = this.radius;
        }
        if (typeof this.props.parent_radius === 'undefined') {
          this.props.parent_radius = this.radius;
        }
      },
      'time': function(time, elapsed, scene, global_time) {
        // if (this.level > 3)  return;
        // temporary added for testing purposes
        // if (this.level > 10) return;
        // this.gradients[0][0] = 1.0 - global_time;
        // this.gradients[1][0] = global_time;
        // this.angle *= 1.005;
        this.opacity = 1. * this.props.opacity;
        if (this.radius > this.props.radius_limit && this.props.spawned === 0) {
          var child_radius = this.radius * 0.67;
          var x = script.video.mode === 1 ?
              (this.props.scale * this.props.parent_radius) - (this.props.scale * child_radius) :
              0;
          this.spawn({
            'id': 'blue_circle',
            // keep somehow the parent radius...
            // // anyway going to render a nice vid anyway
            'x': x,
            'y': 0,
            'radius': child_radius,
            'scale': script.video.mode === 1 ? this.scale : 1.0,
            'props': {
              'parent_radius': this.props.parent_radius,
              'radius_limit': this.props.radius_limit,
              'scale': 1.0,
              'px': this.props.px + x,
              'py': this.props.py
            },

            // cool with #1,2,3
            //'angle': script.video.mode === 1 ? 5 : 0,

            // also cool with #4
            'angle': script.video.mode === 1 ? 15 : 0,
          });
          this.props.spawned++;
          // this.subobj[this.subobj.length - 1].x += ((rand()*2)-1.)*5.;
          // this.subobj[this.subobj.length - 1].y += ((rand()*2)-1.)*5.;
        }

        // awesome! #1
        // if (this.level > 1) this.angle += (elapsed * 5.) * this.level;

        // cool.. #2
        // if (this.level > 1) this.angle += (elapsed * 5.) * (this.level / 10.);

        // awesome! #3
        // if (this.level > 1) this.angle += (elapsed * 5.);

        // ?? # 4
        if (this.level > 1) this.angle += (elapsed * 15.) * this.level;  // * this.level;

        switch (scene) {
          case 0:
            this.radius += elapsed * 100;
            this.props.parent_radius += elapsed * 100;
            // if (this.level > 1 && script.video.mode === 1)
            this.x = (this.props.scale * this.props.parent_radius) - (this.props.scale * this.radius);
        }

        this.opacity = 1.0 * logn(1. - global_time * 1.0, 10000);
        this.opacity *= this.props.opacity
        if (this.opacity <= 0) {
          this.exists = false;
        }
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
