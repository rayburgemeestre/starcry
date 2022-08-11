_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
  },
  'toroidal': {
    'toroidal1': {
      'width': 1920,
      'height': 1080,
    }
  },
  'objects': {
    'snow': {
      'init': function() {
        for (let i = 0; i < 500; i++) this.spawn({'id': 'flake', 'props': {}});
      },
    },
    'flake': {
      'type': 'circle',
      'gradient': 'white',
      'toroidal': 'toroidal1',
      'props': {'size': 1.},
      'radius': 0,
      'radiussize': 50.0,
      'init': function() {
        this.x = (-1920 / 2) + rand() * 1920;
        this.y = (-1080 / 2) + rand() * 1080;
        this.props.size = expf(rand(), 1000);
        this.radiussize = 2. + (3. * this.props.size);
        this.props.x = this.x;
        this.opacity = rand();
      },
      'time': function(t, e, s, tt) {
        this.y += (12.5 * e) + (e * 200 * (this.props.size));
        this.x = this.props.x + (Math.sin(tt * 10) * 200);
      },
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.0,
    'granularity': 1,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [
    {'name': 'scene1', 'duration': 6.0, 'objects': [{'id': 'snow', 'x': 0, 'y': -0, 'z': 0, 'props': {}}]},
  ]
};
