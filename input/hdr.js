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
      'time': function(t, e) {
        script.video.scale = 20 - 19.0 * logn(t, 1000);
        script.video.scale *= 2;
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
        // TODO: - 500, 0, 500, however, note that shape scale currently affects x, y.
        // orb1
        // this.subobj.push({'id': 'blue_circle', 'x': 0, 'y': 0, 'scale': 3.0, 'props': {'radius_limit': 5.,
        // 'opacity': 1.0}}); orb2
        this.subobj.push(
            {'id': 'blue_circle', 'x': 0, 'y': 0, 'scale': 0.75 * 3.0, 'props': {'radius_limit': 20., 'opacity': 1.0}});
        // orb3
        // this.subobj.push(
        //     {'id': 'blue_circle', 'x': 0, 'y': 0, 'scale': 0.5 * 3.0, 'props': {'radius_limit': 50.,
        //     'opacity': 1.0}});
      },
    },
    'blue_circle': {
      'type': 'circle',
      'gradients': [
        [1.0, 'blue'],
        [0.0, 'green'],
      ],
      'texture': 'clouds',
      'radius': 100.,
      'radiussize': 30.0,
      'opacity': 0.,
      'blending_type': blending_type.pinlight,
      'angle': 1,
      'props': {},
      'scale': 1.0,
      'init': function() {},
      'time': function(time, elapsed) {
        this.gradients[0][0] = 1.0 - time;
        this.gradients[1][0] = time;
        this.opacity = 1. * this.props.opacity;
        if (this.radius > this.props.radius_limit && this.subobj.length == 0) {
          var child_radius = this.radius * 0.67;
          this.subobj.push({
            'id': 'blue_circle',
            'x': this.radius - child_radius,
            'y': 0,
            'radius': child_radius,
            'scale': this.scale,
            'props': this.props,
            'angle': 1,
          });
        }
        if (this.level > 1) this.angle += elapsed * 5.;
        if (time > 0.5) {
          this.radius += elapsed * 100;
        }
        this.opacity = 1.0 * logn(1. - time * 1.1, 1000);
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
    'duration': 5,
    'fps': 25,
    'width': 1920 * 2.,
    'height': 1920 * 2.,
    'scale': 10. * 2.,
    'granularity': 1,
    'grain_for_opacity': true,
    'motion_blur': true,
    'max_intermediates': 30.,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'camera', 'x': 0, 'y': 0},
      {'id': 'mother', 'x': 0, 'y': 0},
    ],
  }]
};
