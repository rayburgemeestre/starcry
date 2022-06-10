_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
      {'position': 0.8, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
    ],
  },
  'objects': {
    'ball': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 0,
      'x': 0,
      'y': 0,
      'radiussize': 20.0,
      'props': {'grad': 'white', 'mode': 'linear'},
      'blending_type': blending_type.normal,
      'init': function() {},
      'time': function(t, elapsed) {
        let linear = Math.min(1.0, t * 2.0);
        if (this.props.mode === 'linear') {
          this.x = 1400 * linear;
        } else if (this.props.mode === 'expf') {
          this.x = 1400 * expf(linear, 50000);
        } else if (this.props.mode === 'logn') {
          this.x = 1400 * logn(linear, 50000);
        }
        this.x -= 700;
      },
    },
  },
  'video': {
    'fps': 30,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 1,
    'granularity': 1,
    'bg_color': {'r': 0, 'g': 0, 'b': 0, 'a': 1},
    'minimize_steps_per_object': true,  // this guy is interesting to debug!!
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 3,
    'objects': [
      //      {'id': 'ball', 'x': -700, 'y': -300, 'z': 0, 'props': {'mode': 'linear'}},
      {'id': 'ball', 'x': -700, 'y': 0, 'z': 0, 'props': {'mode': 'expf'}},
      //      {'id': 'ball', 'x': -700, 'y': 300, 'z': 0, 'props': {'mode': 'logn'}},
    ],
  }]
};
