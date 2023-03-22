_ = {
  'gradients': {
    'red': '#ff0000@0.9',
    'white': '#ffffff@0.9',
    'text': '#ff0000@0.0',
  },
  'textures': {
    'text0': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 6,
      'persistence': 0.50,
      'percentage': 1.0,
      'scale': 50.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'rotation': 0.0,  // TODO: !
      'speed': 1.,
    },
    'perlin': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 7,
      'persistence': 0.45,
      'percentage': 1.0,
      'scale': 100.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'text1b': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 7,
      'persistence': 0.45,
      'percentage': 1.0,
      'scale': 10.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'fractal': {
      'type': 'fractal',
      'size': 3000.,
      'octaves': 3,
      'persistence': 0.50,
      'percentage': 0.8,
      'scale': 75.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'turbulence': {
      'type': 'turbulence',
      'size': 3000.,
      'octaves': 4,
      'persistence': 0.50,
      'percentage': 0.8,
      'scale': 50 / 2,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'turbulence2': {
      'type': 'turbulence',
      'size': 100.,
      'octaves': 4,
      'persistence': 0.50,
      'percentage': 0.8,
      'scale': 50 / 2,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'text4': {
      'type': 'turbulence',
      'size': 200.,
      'octaves': 4,
      'persistence': 0.50,
      'percentage': 0.8,
      'scale': 10,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
  },
  'objects': {

    'main': {
      'init': function() {
        let textures = [
          'perlin',
          'fractal',
          'turbulence',
          'turbulence2',
        ];
        let perlin_texture_types = [
          texture_3d.raw,
          texture_3d.radial_displacement,
          texture_3d.radial_compression,
          texture_3d.radial_distortion,
          texture_3d.radial_scaling,
          texture_3d.spherical,
          texture_3d.noise_3d_simplex,
          texture_3d.noise_3d_coords,
        ];
        let offset_x = 1920 / 2;
        let offset_y = 1080 / 2;
        let radius = 120;
        let x = radius;
        let y = radius;
        for (let txt of textures) {
          for (let type of perlin_texture_types) {
            this.spawn({
              'id': 'obj',
              'x': x - offset_x,
              'y': y - offset_y,
              'z': 0,
              'radiussize': radius,
              'texture': txt,
              'seed': 2,
              'texture_3d': type,
              'attrs': {'text': txt + '@' + texture_3d_str(type)}
            });

            x += radius * 2;
            if (x > (1920 - radius)) {
              x = radius;
              y += (1080 / 4.);  // radius * 2;
            }
          }
        }
      }
    },
    'obj': {
      'type': 'circle',
      'blending_type': blending_type.normal,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 50,
      'props': {},
      'init': function() {
        let [texture, mode] = this.attr('text').split('@');
        this.spawn({
          'id': 'text',
          'y': 125,
          'text': mode,
        });
        this.spawn({
          'id': 'text',
          'y': 145,
          'text': texture,
        });
      },
    },
    'text': {
      'type': 'text',
      'gradient': 'text',
      'text': '',
      'text_size': 20,
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
    'update_positions': false,
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
        'id': 'main',
        'x': 0,
        'y': 0,
        'z': 0,
        'texture': 'text1',
        'seed': 2,
        'texture_3d': texture_3d.disabled,
        'attrs': {'text': 'perlin'}
      },
    ],
  }]
};
