_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
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
      'init': function() {},
      'time': function(t, elapsed) {
        let max = 50;
        let maxexp = Math.log(max + 1.0) / Math.log(2.0);

        let linear = Math.min(1.0, t * 2.0);
        let expf = ((Math.pow(2.0, (linear) * maxexp)) - 1.0) / max;

        let maxpow = Math.pow(2.0, maxexp);
        let logn = (maxpow - (Math.pow(2.0, (1.0 - linear) * maxexp))) / max;
        if (this.props.mode === 'linear') {
          this.x = 1400 * linear;
        } else if (this.props.mode === 'expf') {
          this.x = 1400 * expf;
        } else if (this.props.mode === 'logn') {
          this.x = 1400 * logn;
        }
        this.x -= 700;
      },
    },
  },
  'video': {
    'duration': 3,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 1,
    'granularity': 1,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'ball', 'x': -700, 'y': -300, 'z': 0, 'props': {'mode': 'linear'}},
      {'id': 'ball', 'x': -700, 'y': 0, 'z': 0, 'props': {'mode': 'expf'}},
      {'id': 'ball', 'x': -700, 'y': 300, 'z': 0, 'props': {'mode': 'logn'}},
    ],
  }]
};
