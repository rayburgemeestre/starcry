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
      'scale': (50 / 2),
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
      'radiussize': 10,
      'props': {},
      'init': function() {
        this.spawn({
          'id': 'text',
          'y': 125,
          'text': this.attr('text'),
        });
      },
      'time': function(t, e, s, tt) {
        // this.x += 10;
        this.radiussize += 1;
      }
    },
    'text': {
      'type': 'text',
      'gradient': 'text',
      'text': '',
      'text_size': 30,
      'text_align': 'center',
      'text_fixed': false,
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
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
        'x': -500,
        'y': 0,
        'z': 0,
        'texture': 'turbulence2',
        'seed': 3,
        'texture_3d': texture_3d.radial_compression,
        'attrs': {'text': 'asdf'}
      },

      {
        'id': 'obj',
        'x': 500,
        'y': 0,
        'z': 0,
        'texture': 'turbulence2',
        'seed': 3,
        'texture_3d': texture_3d.radial_compression,
        'attrs': {'text': 'asdf'}
      },

    ],
  }]
};
