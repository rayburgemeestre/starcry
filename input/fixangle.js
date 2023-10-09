_ = {
  'gradients': {
    'green': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'mother': {
      'x': 0,
      'y': 0,
      'angle': 0,
      'opacity': 1,  // TEMP HACK
      'init': function() {
        this.spawn({
          'id': 'test_circle',
          'x': -500 + 50.,
          'y': 0,
          'scale': 0.5,
          'props': {'radius_limit': 5., 'x': -500 + 50.}
        });
        this.spawn({'id': 'test_circle', 'x': 0 + 50., 'y': 0, 'scale': 1.0, 'props': {'radius_limit': 5., 'x': 50.}});
        this.spawn(
            {'id': 'test_circle', 'x': 500 + 50., 'y': 0, 'scale': 1.0, 'props': {'radius_limit': 5., 'x': 500 + 50.}});
      },
    },
    'test_circle': {
      'type': 'circle',
      'gradient': 'green',
      'radius': 200.,
      'radiussize': 50.0,
      'opacity': 1.,
      'blending_type': blending_type.pinlight,
      'angle': 0,
      'props': {'depth': 1},
      'scale': 1.0,
      'init': function() {},
      'time': function(time, elapsed) {
        var total_frames = script.video.duration * 25. - 1;
        var current_frame = total_frames * time;
        // output("initialized with x: " + this.x + " passed to triang: " + triangular_wave(fr, 1., 25.)  + " fr = " +
        // fr + " el=" + elapsed);

        // this.x = this.props.x + triangular_wave(current_frame, 1., 25.) * 5.;
        this.x = this.props.x + triangular_wave(current_frame, 1., 1.0) * 50.;

        //        output("initialized with x: " + this.x + " passed to triang: " +
        //        triangular_wave(current_frame, 1., 25.)  + " fr = " + current_frame + " el=" + total_frames);

        this.opacity = 1.;
        // return;
        if (this.props.depth < 10. && this.attr('flag') != 1) {
          this.set_attr('flag', 1);
          var child_radius = this.radius * 0.67;
          var child_x = this.radius - child_radius + 50.;
          this.spawn({
            'id': 'test_circle',
            'x': child_x,
            'y': 0,
            'radius': child_radius,
            'scale': this.scale,
            'props': {'depth': this.props.depth + 1, 'x': child_x},
            'angle': 1,
          });
        }
        if (this.level > 1) this.angle += elapsed * 5.;
      },
    },
    'green_circle': {
      'type': 'circle',
      'gradient': 'green',
      'radius': 0.,
      'radiussize': 30.0,
      'blending_type': blending_type.pinlight,
      'init': function() {},
      'time': function(time, elapsed) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 1.,
    'granularity': 1,
    'grain_for_opacity': true,
    'motion_blur': true,
    'min_intermediates': 2,
    'max_intermediates': 2,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 5,
    'objects': [
      // {'id': 'green_circle', 'x': -500, 'y': 0, },
      // {'id': 'green_circle', 'x': 0, 'y': 0},
      // {'id': 'green_circle', 'x': 500, 'y': 0},
      {'id': 'mother', 'x': 0, 'y': 0},
    ],
  }]
};
