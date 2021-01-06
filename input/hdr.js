_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.1, 'r': 0, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.1, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
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
        script.video.scale = 10 - 9.0 * logn(gt, 1000);
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
      'subobj': [],
      'x': 0,
      'y': 0,
      'angle': 0,
      'init': function() {
        // temporary changed for testing purposes
        if (false)
          this.subobj.push({
            'id': 'blue_circle',
            'x': -450,
            'y': 0,
            'scale': 0.5,
            'props': {'radius_limit': 60., 'opacity': 1.0}
          });  // 50.
        if (false)
          this.subobj.push({
            'id': 'blue_circle',
            'x': 0,  // - 100,
            'y': 0,
            'scale': 0.75,
            'props': {'radius_limit': 60., 'opacity': 1.0}
          });  // 20.
        this.subobj.push({
          'id': 'blue_circle',
          //'x': 550,
          'x': 0,
          'y': 0,
          'scale': 1.0,
          'props': {'radius_limit': 5., 'opacity': 1.0, 'parent': false}
        });  // was 5.
      },
    },
    'blue_circle': {
      'type': 'circle',
      'gradients': [
        [1.0, 'blue'],
        [0.0, 'green'],
      ],
      // temporary commentedfor testing purposes
      'texture': 'clouds',
      'radius': 100.,
      'radiussize': 30.0,
      'opacity': 0.,
      'blending_type': blending_type.pinlight,
      'angle': 1,
      'props': {
        'parent': false,
        'extra_radius': 0,
      },
      'scale': 1.0,
      'init': function() {
        if (this.props.parent === false) {
          this.props.parent = this;
        }
      },
      'time': function(time, elapsed, scene, global_time) {
        // temporary added for testing purposes
        // if (this.level > 10) return;
        this.gradients[0][0] = 1.0 - global_time;
        this.gradients[1][0] = global_time;
        this.opacity = 1. * this.props.opacity;
        if (this.radius > this.props.radius_limit && this.subobj.length == 0 && true) {
          var child_radius = this.radius * 0.67;
          this.subobj.push({
            'id': 'blue_circle',
            // keep somehow the parent radius...
            // // anyway going to render a nice vid anyway
            'x': this.radius - child_radius,
            'y': 0,
            'radius': child_radius,
            'scale': this.scale,
            'props': this.props,
            'angle': 1,
          });
        }
        if (this.level > 1) this.angle += elapsed * 5.;

        switch (scene) {
          case 0:
            break;
          case 1:
            this.radius += elapsed * 100;
            this.x += elapsed * 1;
        }

        this.opacity = 1.0 * logn(1. - global_time * 1.0, 10000);
        this.opacity *= this.props.opacity
        if (this.opacity <= 0) {
          this.exists = false;
        }
      },
      'on': {
        'collide': function(other) {
          this.props.state = 'growing';
          other.props.state = 'growing';
        }
      }
    },
  },
  'video': {
    //'duration': 5,
    'fps': 25,
    //'width': 1920 * 2.,
    //'height': 1920 * 2.,
    //'scale': 10. * 2.,
    'width': 1920,
    'height': 1920,
    'scale': 10.,
    'granularity': 1,
    'grain_for_opacity': true,
    'motion_blur': true,
    'max_intermediates': 30.,
    'perlin_noise': true,
    'dithering': true,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 1,
      'objects': [
        {'id': 'camera', 'x': 0, 'y': 0},
        {'id': 'mother', 'x': 0, 'y': 0},
      ],
    },
    {
      'name': 'scene2',
      'duration': 4,
      'objects': [
        {'id': 'camera', 'x': 0, 'y': 0},
        {'id': 'mother', 'x': 0, 'y': 0},
      ],
    }
  ]
};
