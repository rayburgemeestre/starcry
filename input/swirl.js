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
        //        script.video.scale = 20 - 19.0 * logn(t, 1000);
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
      'x': 0,
      'y': 0,
      'angle': 0,
      'init': function() {
        for (var i = 0; i < 6.; i++) {
          var x = script.video.width * rand() - (script.video.width / 2.);
          var y = script.video.height * rand() - (script.video.height / 2.);
          x = 0;
          y = 0;
          this.spawn({
            'id': 'blue_circle',
            'x': x,
            'y': y,
            'hue': (i / 10.) * 360.,
            'scale': 1. + rand(),
            'props': {'radius_limit': rand() * 10. + 5., 'hue': (i / 10.) * 180.}
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
      'hue': 0.,
      'radiussize': 30.0,
      'opacity': 0.,
      //'blending_type': blending_type.normal,
      //'blending_type': blending_type.pinlight,
      //'blending_type': blending_type.phoenix,
      'blending_type': blending_type.add,
      'angle': 0,
      'props': {},
      'scale': 1.0,
      'init': function() {
        // this.opacity = 1; // temporary measure (see below TODO)
        this.opacity = 0.5;  // temporary measure (see below TODO)
        this.opacity = 1.0;  // temporary measure (see below TODO)
        output('radius = ' + this.radius * this.scale);
        if (this.radius * this.scale < 20) {
          this.opacity = 0;
        }
        this.set_attr('flag', 1);
        this.hue = this.props.hue;
        this.props.radius = this.radius;
        this.props.offset = rand();
      },
      'time': function(time, elapsed) {
        this.opacity = 1.;  // TODO: investigate, why is this not doing anything?
        this.gradients[0][0] = 1.0 - time;
        this.gradients[1][0] = time;

        this.radius = this.props.radius + Math.cos(this.props.offset * time * 10) * 10;
        this.hue += elapsed * 20;

        if (false)
          if (this.radius > this.props.radius_limit && this.attr('flag') == 1) {
            this.set_attr('flag', 2);
            this.spawn({
              'id': 'blue_circle',
              'x': this.radius / 3.,
              'y': 0,
              'radius': this.radius * 0.67,
              'scale': this.scale,
              'props': this.props
            });
          }
        // this.rotate += elapsed * 5.;
        // return;

        return;  //
                 //
        if (time > 0.5) {
          // this.radius += elapsed * 100;
        }
        this.opacity = 1.0 * logn(1. - time * 1.1, 1000);
        if (this.opacity <= 0) {
          output('stop with existing');
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
    //'duration': 10,//5,
    'fps': 25,
    //    'width': 1920,
    //    'height': 1920,
    'width': 3840,
    'height': 2160,
    //    'scale': 10. /*was 20*/,
    //'scale': 1. /*was 20*/,
    //'scale': 3.0 /*was 20*/,
    //'scale': 2.5 /*was 20*/,
    'scale': 2.0 /*was 20*/,
    'granularity': 1,
    'grain_for_opacity': false,
    'min_granularity': 2,
    'max_granularity': 2,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 30.0,
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
