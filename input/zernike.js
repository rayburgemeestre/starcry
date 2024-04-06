script = {
  'gradients': {
    'grad1': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'white': '#ffffff@0.9',
    'text': '#ff0000@0.0',
  },
  'textures': get_textures(),
  'objects': {

    'main': {
      'init': function() {
        let offset_x = 1920 / 2;
        let offset_y = 1080 / 2;
        let radius = 120;
        let x = radius;
        let y = radius;

        for (let i = 0; i < 21 + 11; i++) {
          this.spawn({
            'id': 'obj',
            'x': x - offset_x,
            'y': y - offset_y,
            'z': 0,
            'radiussize': radius,
            'texture': 'zern' + (i + 1),
            'zernike_type': zernike_type.version1,
            'attrs': {
              'text': 'zernike #' + (i + 1) + ' n=' + script.textures['zern' + (i + 1)].n +
                  ',m=' + script.textures['zern' + (i + 1)].m + ''
            },
          });
          x += radius * 2;
          if (x > (1920 - radius)) {
            x = radius;
            y += (1080 / 4.);
          }
        }
      }
    },
    'obj': {
      'type': 'circle',
      'blending_type': blending_type.normal,
      'gradient': 'grad1',
      'radius': 0,
      'radiussize': 50,
      'init': function() {
        let text = this.attr('text');
        this.spawn({'id': 'text', 'y': 105, 'text': text});
      },
    },
    'text': {
      'type': 'text',
      'gradient': 'text',
      'text': '',
      'text_size': 20,
      'text_align': 'center',
      'text_fixed': false,
      'time': function(t, e, s, T) {
        if (this.attr('text')) this.text = this.attr('text');
      }
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
    // 'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'minimize_steps_per_object': false,
    'grain_for_opacity': false,
    'min_intermediates': 1,
    'max_intermediates': 1,
  },
  'scenes': [
    {
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
    },
    {
      'name': 'scene2',
      'duration': 5,
      'objects': [],
    }
  ]
};

function get_textures() {
  let nmPairs = [];
  let totalPairs = 21 + 11 + 1;
  for (let n = 0; nmPairs.length < totalPairs; n++) {
    for (let m = -n; m <= n; m += 2) {
      if ((n - Math.abs(m)) % 2 === 0) {  // if n - |m| is even
        nmPairs.push([n, m]);
        if (nmPairs.length >= totalPairs) {
          break;  // reached desired num of pairs
        }
      }
    }
  }

  let ret = {};
  let i = 0;
  for (let pair of nmPairs) {
    ret['zern' + i++] = {
      'type': 'zernike',
      'n': pair[0],
      'm': pair[1],
      'zernike_type': zernike_type.version1,
      'effect': texture_effect.color
    };
  }
  return ret;
}

script;
