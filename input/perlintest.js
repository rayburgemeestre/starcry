_ = {
  'gradients': {
    'red': '#ff0000@0.9',
    'white': '#ffffff@0.9',
    'text': '#ff0000@0.0',
  },
  'textures': {
    'turbulence2': {
      'type': 'turbulence',
      'size': 100.,
      'octaves': 4,
      'persistence': 0.50,
      'percentage': 0.8,
      'scale': 25,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
  },
  'objects': {
    'obj': {
      'type': 'circle',
      'blending_type': blending_type.normal,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 50,
      'props': {},
      'texture': 'turbulence2',
      'texture_3d': texture_3d.radial_scaling,
      'texture_offset_x': 100,
      'texture_offset_y': 0,
      'init': function() {},
      'time': function(t, e, s, tt) {
        this.radiussize = 50 + (t * 200);
      }
    },
  },
  'video': {
    'fps': 25,
    // 'width': 1920,
    // 'height': 1080,
    'width': 1920 / 2,
    'height': 1080 / 2,
    'scale': 1.0,
    'init_scale': 0.5,
    'rand_seed': 1,
    'granularity': 100,
    // 'update_positions': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'minimize_steps_per_object': false,
    'grain_for_opacity': false,
    'min_intermediates': 1,
    'max_intermediates': 1,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 10,
    'objects': [
      {
        'id': 'obj',
        'x': -300,
        'y': 0,
        'z': 0,
        'seed': 2,
      },
      {
        'id': 'obj',
        'x': 300,
        'y': 0,
        'z': 0,
        'scale': 2.,
        'seed': 2,
      },
    ],
  }]
};
