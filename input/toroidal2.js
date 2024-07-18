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
      'width': 500.,  // means -250 to 250
      'height': 500,  // means -250 to 250
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
      'radiussize': 10.0,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, s) {
        // transition to zooming out
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
    'width': 500,
    'height': 500,
    'scale': 1.,
    'scale_ratio': false,
    'rand_seed': 5,
    'granularity': 1,
    // 'min_intermediates': 5.,
    // 'max_intermediates': 50.,
    'min_intermediates': 2,
    'max_intermediates': 2,
    'grain_for_opacity': true,
    'bg_color': {r: 0, g: 0, b: 0, a: 1},
  },
  'preview': {
    'max_intermediates': 1.,
  },
  'scenes': [
    {
      'name': 'horizontal warp',
      'duration': 2.,
      'objects': [
        // {'id': 'box', 'x': 0, 'y': 0, 'z': 0},
        {'id': 'circle', 'x': 250, 'y': 0, 'z': 0, 'velocity': 0, 'vel_x': 0, 'vel_y': 0, 'props': {'s': 1}},
      ]
    },
    //   {
    //     'name': 'vertical warp',
    //     'duration': 2.,
    //     'objects': [
    //       {'id': 'circle', 'x': 0, 'y': 700, 'z': 0, 'velocity': 1, 'vel_x': 0, 'vel_y': -10, 'props': {'s': 2}},
    //     ]
    //   },
    //   {
    //     'name': 'diagonal warp',
    //     'duration': 2.,
    //     'objects': [
    //       {'id': 'circle', 'x': 700, 'y': 700, 'z': 0, 'velocity': 1, 'vel_x': -10, 'vel_y': -10, 'props': {'s': 3}},
    //     ]
    //   },
    //   {
    //     'name': 'transition',
    //     'duration': 0.5,
    //     'objects': [
    //       {'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'velocity': 1, 'vel_x': -10, 'vel_y': -10, 'props': {'t': 2}},
    //     ]
    //   },
    //   {
    //     'name': 'horizontal warp - zoom out',
    //     'duration': 2.,
    //     'objects': [
    //       {
    //         'id': 'circle',
    //         'label': 'circle2',
    //         'x': 700,
    //         'y': 0,
    //         'z': 0,
    //         'velocity': 1,
    //         'vel_x': -10,
    //         'vel_y': 0,
    //         'props': {'s': 5}
    //       },
    //       {'id': 'box', 'x': 0, 'y': 0, 'z': 0},
    //     ]
    //   },
    //   {
    //     'name': 'vertical warp - zoom out',
    //     'duration': 2,
    //     'objects': [
    //       {
    //         'id': 'circle',
    //         'label': 'circle3',
    //         'x': 0,
    //         'y': 700,
    //         'z': 0,
    //         'velocity': 1,
    //         'vel_x': 0,
    //         'vel_y': -10,
    //         'props': {'s': 6}
    //       },
    //       {'id': 'box', 'x': 0, 'y': 0, 'z': 0},
    //     ]
    //   },
    //   {
    //     'name': 'diagonal warp - zoom out',
    //     'duration': 2,
    //     'objects': [
    //       {'id': 'circle', 'x': 700, 'y': 700, 'z': 0, 'velocity': 1, 'vel_x': -10, 'vel_y': -10, 'props': {'s': 7}},
    //       {'id': 'box', 'x': 0, 'y': 0, 'z': 0},
    //     ]
    //   },
  ]
};