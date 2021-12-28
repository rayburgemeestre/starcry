_ = {
  'gradients': {
    'wit': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      'width': 1920,
      'height': 1080,
    }
  },
  'objects': {
    'sneeuw': {
      'x': 0,
      'y': 0,
      'subobj': [],
      'init': function() {
        for (let i = 0; i < 500; i++) {
          this.subobj.push({'id': 'vlokje', 'props': {}})
        }
      },
      'time': function(t, e, scene) {},
    },
    'vlokje': {
      'type': 'circle',
      'gradient': 'wit',
      'toroidal': 't1',
      'x': 0,
      'y': 0,
      'props': {'size': 1.},
      'subobj': [],
      'radius': 0,
      'radiussize': 50.0,
      'angle': 0.,
      'init': function() {
        this.x = (-1920 / 2) + rand() * 1920;
        this.y = (-1080 / 2) + rand() * 1080;
        this.props.size = expf(rand(), 1000);
        this.radiussize = 2. + (3. * this.props.size);
        this.props.x = this.x;
        this.opacity = rand();
      },
      'time': function(t, e, scene, tt) {
        this.y += 0.5 + (e * 200 * (this.props.size));
        this.x = this.props.x + (Math.sin(tt * 10) * 200);
      },
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.0,
    'rand_seed': 5,
    'granularity': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': false,
    'motion_blur': false,
  },
  'preview': {
    'max_intermediates': 1.,
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {'name': 'scene1', 'duration': 60.0, 'objects': [{'id': 'sneeuw', 'x': 0, 'y': -0, 'z': 0, 'props': {}}]},
  ]
};
