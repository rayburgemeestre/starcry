_ = {
  'gradients': {
    'white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.5, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      // 'x': -100,
      // 'y': -100,
      'width': 500,
      'height': 500,
    }
  },
  'objects': {
    'circle': {
      'type': 'circle',
      'gradient': 'white',
      'toroidal': 't1',
      'blending_type': blending_type.normal,
      'x': 0,
      'y': 0,
      'props': {'t': 1},
      'subobj': [],
      'radius': 0,
      'radiussize': 80.0,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, s) {
        // // transition to zooming out
        // if (this.props.t == 2) {
        //   script.video.scale = 1.;
        // }
        // this.opacity = s + 1 === this.props.s ? 1.0 : 0.;
      },
    },
    'box': {
      'x': 0,
      'y': 0,
      'subobj': [],
      'init': function() {
        this.spawn({'id': 'line', 'x': -(250), 'y': -(250), 'x2': (250), 'y2': -(250)});
        this.spawn({'id': 'line', 'x': -(250), 'y': -(250), 'x2': -(250), 'y2': (250)});
        this.spawn({'id': 'line', 'x': -(250), 'y': (250), 'x2': (250), 'y2': (250)});
        this.spawn({'id': 'line', 'x': (250), 'y': (250), 'x2': (250), 'y2': -(250)});
      },
      'time': function() {},
    },
    'line': {
      'type': 'line',
      'gradient': 'white',
      'radiussize': 5.,
      'angle': 0.,
      'init': function() {},
      'time': function() {},
    },
  },
  'video': {
    'fps': 30,
    'width': 1000,
    'height': 1000,
    'scale_ratio': false,
    // 'scale': 1.0,
    'scale': 0.5,
    'rand_seed': 5,
    'granularity': 1,
    'grain_for_opacity': true,
    'bg_color': {r: 0, g: 0, b: 0, a: 1},
  },
  'preview': {
    'max_intermediates': 1.,
  },
  'scenes': [
    {
      'name': 'horizontal warp - zoom out',
      'duration': 2.,
      'objects': [
        {
          'id': 'circle',
          'label': 'circle2',
          'x': -100,
          'y': -100,
          'z': 0,
          'velocity': 10,
          'vel_x': -3,
          'vel_y': -5,
          'props': {'s': 5}
        },
        {'id': 'box', 'x': -100, 'y': -100, 'z': 0},
      ]
    },
  ]
};