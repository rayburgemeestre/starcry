_ = {
  'gradients': {
    'white': '#ffffff',
    'red': '#ff0000',
  },
  'objects': {
    'dot': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 10.0,
      'gradient': 'white',
      'opacity': 0.5,
      'hue': 0.,
      'rotate': 0.,
      //'unique_group': 'group1',
      'blending_type': blending_type.normal,
      'init': function() {
        if (this.level > 4) {
          this.opacity = 1;
          return;
        }
        let stepsize = 80;
        let vec = [
          [stepsize, 0],
          [0, stepsize],
          [-stepsize, 0],
          [0, -stepsize],
        ];
        for (let v of vec) {
          this.spawn({'id': 'dot', 'x': v[0], 'y': v[1]});
        }
      },
      'time': function(t, elapsed, s) {
        this.rotate += elapsed * 80;
        // this.rotate += elapsed * 200;
        // this.rotate += elapsed * 20;
      },
    },
  },
  'video': {
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'min_intermediates': 1,
    'max_intermediates': 10,
    'width': 1900,
    'height': 1900,
    'scale': 1.00,
  },
  'scenes': [
    {
      'name': 'scene',
      'duration': 10.0,
      'objects': [
        // {'id': 'dot', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'dot', 'x': -400, 'y': 0, 'z': 0},
        {'id': 'dot', 'x': 400, 'y': 0, 'z': 0, 'gradient': 'red', 'unique_group': 'group1'},
      ]
    },
  ]
};
