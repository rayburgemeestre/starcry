_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.3, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.6, 'r': 1, 'g': 1, 'b': 1, 'a': 0.8},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'textures': {},
  'objects': {
    'obj': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 0,
      'radiussize': 5.0,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {
        this.radius += elapsed * 50;
        if (this.radius >= 120.) {
          this.exists = false;
        }
        if (this.radius >= 50. && this.subobj.length == 0) {
          this.subobj.push({'id': 'obj', 'label': 'sub1', 'x': 0, 'y': 0, 'z': 0, 'props': {}});
        }
      },
    },
  },
  'video': {
    'duration': 5,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.,
    'rand_seed': 1,
    'granularity': 1,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'obj', 'label': 'first', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
