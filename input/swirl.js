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
        // script.video.scale *= 2.;// for the still
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
        for (var i = 0; i < 10.; i++) {
          var x = script.video.width * rand() - (script.video.width / 2.);
          var y = script.video.height * rand() - (script.video.height / 2.);
          this.subobj.push({
            'id': 'blue_circle',
            'x': x,
            'y': y,
            'scale': rand() * 3.,
            'props': {'radius_limit': rand() * 10. + 5.}
          });
        }
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
      'angle': 0,
      'props': {},
      'scale': 1.0,
      'init': function() {},
      'time': function(time, elapsed) {
        this.gradients[0][0] = 1.0 - time;
        this.gradients[1][0] = time;

        this.opacity = 1.;
        // return;
        if (this.radius > this.props.radius_limit && this.subobj.length == 0) {
          this.subobj.push({
            'id': 'blue_circle',
            'x': this.radius / 3.,
            'y': 0,
            'radius': this.radius * 0.67,
            'scale': this.scale,
            'props': this.props
          });
        }
        this.angle += elapsed * 5.;
        if (time > 0.5) {
          this.radius += elapsed * 100;
        }
        this.opacity = 1.0 * logn(1. - time * 1.1, 1000);
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
    // 'green_circle': {
    //   'type': 'circle',
    //   'gradient': 'green',
    //   'radius': 0.,
    //   'radiussize': 30.0,
    //   'blending_type': blending_type.pinlight,
    //   'props': { 'state': 'default', },
    //   'collision_group': 'one',
    //   'init': function() {
    //   },
    //   'time': function(time, elapsed) {
    //     switch (this.props.state) {
    //       case 'default':
    //         this.velocity = 1. + 100. * expf(time, 10.);
    //         break;
    //       case 'growing':
    //         this.radius += elapsed * 100.;
    //         break;
    //     }
    //   },
    //   'on': {
    //     'collide': function(other) {
    //       this.props.state = 'growing';
    //       other.props.state = 'growing';
    //     }
    //   }
    // },
  },
  'video': {
    'duration': 5,
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 10. /*was 20*/,
    'granularity': 1,
    'grain_for_opacity': true
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'camera', 'x': 0, 'y': 0, 'z': 0},
      // {'id': 'background', 'x': 0, 'y': 0, 'z': 0},
      //{'id': 'green_circle', 'x': -100, 'y': 1, 'vel_x': 1, 'vel_y': 0, 'velocity': 1.},
      //{'id': 'green_circle', 'x': 100, 'y': -1, 'vel_x': -1, 'vel_y': 0, 'velocity': 1.},
      //      {'id': 'blue_circle', 'x': 0, 'y': 0, 'props': {'offset_angle': 0.} },
      //      {'id': 'blue_circle', 'x': 300, 'y': 0, 'scale': 0.5,  'props': {'offset_angle': 30.} },
      {'id': 'mother', 'x': 0, 'y': 0},
    ],
  }]
};
