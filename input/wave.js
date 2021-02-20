_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'test_circle': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 150.,
      'radiussize': 5.0,
      'opacity': 1.,
      'blending_type': blending_type.pinlight,
      'angle': 0,
      'props': {'mode': 1},
      'subobj': [],
      'scale': 1.0,
      'init': function() {
        this.props.wave_strength = 0.;
        this.props.x = this.x;
        this.subobj.push({'id': 'test_ball', 'x': 0, 'y': 0});
      },
      'time': function(time, elapsed) {
        // TODO: hide this complexity (of - 1 and calculating total/current frames) in a more "native" vibe/wave
        // function in C++
        // EDIT2: perhaps wasn't a good idea after all.
        var total_frames = script.video.duration * 25. /* DISABLED AGAIN! - 1.*/;
        var current_frame = total_frames * time;
        // output('vibe current_frame: ' + current_frame);
        // output('vibe: ' + triangular_wave(current_frame, 1., 1.0));
        if (this.props.mode == 1) {
          var q = current_frame - parseInt(current_frame);
          this.x = this.props.x + (q * 600. * this.props.wave_strength) - (300 * this.props.wave_strength);
        } else if (this.props.mode == 2) {
          this.x = this.props.x + triangular_wave(current_frame, 1., 1.0) * 300. * this.props.wave_strength;
        } else if (this.props.mode == 3) {
          this.x = this.props.x + (-1 * triangular_wave(current_frame, 1., 1.0)) * 300. * this.props.wave_strength;
        } else if (this.props.mode == 4) {
          this.x = this.props.x + triangular_wave(current_frame, 1., 1.0) * 300. * this.props.wave_strength;
        }
        this.props.wave_strength = time;
        this.props.prev_time = time;
      },
    },
    'test_ball': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 0.,
      'radiussize': 50.0,
      'opacity': 1.,
      'blending_type': blending_type.pinlight,
      'angle': 0,
      'props': {'mode': 1},
      'scale': 1.0,
      'init': function() {},
      'time': function(time, elapsed) {},
    },
  },
  'video': {
    'duration': 5,
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 1.,
    'granularity': 1,
    'grain_for_opacity': false,
    'motion_blur': true,
    'min_intermediates': 10.,
    'max_intermediates': 150.,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'test_circle', 'x': 0, 'y': -750, 'props': {'mode': 1}},
      {'id': 'test_circle', 'x': 0, 'y': -250, 'props': {'mode': 2}},
      {'id': 'test_circle', 'x': 0, 'y': 250, 'props': {'mode': 3}},
      {'id': 'test_circle', 'x': 0, 'y': 750, 'props': {'mode': 4}},
    ],
  }]
};
