_ = {
  'gradients': {
    'grey': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'rgb-1': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.333, 'r': 0, 'g': 1, 'b': 0, 'a': 1},
      {'position': 0.666, 'r': 0, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ]
  },
  'textures': {
    'zern1': {
      'type': 'zernike',
      // 'n': 111,
      // 'm': -4,
      // 'n': 3,
      // 'm': 1,
      'n': 10,
      'm': -5,
      'zernike_type': zernike_type.version1,
      'test': 12345,
      'debug': '?:' + zernike_type.version1,
      // 'rho': 3,
      // 'theta': 3,
    },
    'zern2': {
      'type': 'zernike',
      'n': 111,
      'm': -4,
      // 'n': 3,
      // 'm': 1,
      'zernike_type': zernike_type.version2,
      'test': 12345,
      'debug': '?:' + zernike_type.version2,
      // 'rho': 3,
      // 'theta': 3,
    },
  },
  'objects': {
    'circle1': {
      'type': 'circle',
      'texture': 'zern1',
      'radius': 0,
      'radiussize': 350.0,
      'gradient': 'grey',
      'init': function() {
        // output("script is: " + script.textures['zern1'].n);
        output('debug1 = ' + zernike_type.version2);
        output('debug2 = ' + zernike_type.version2);
      },
      'time': function() {
        // script.textures['zern1'].n = Math.random() * 10;
        // script.textures['zern1'].m = Math.random() * 10;
        // trigger re-initialization of the texture somehow?
      },
    },
  },
  'video': {
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': false,
    'gamma': 1.0,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 10.0,
      'objects': [
        {'id': 'circle1', 'x': -400, 'y': 0, 'z': 0},
        {'id': 'circle1', 'x': 400, 'y': 0, 'z': 0, 'texture': 'zern2'},
      ]
    },
  ]
};
