_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0.3333, 'b': 0.3333, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0.3333, 'b': 0.3333, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 0.3333, 'g': 1, 'b': 0.3333, 'a': 1},
      {'position': 1.0, 'r': 0.3333, 'g': 1, 'b': 0.3333, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0.3333, 'g': 0.3333, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0.3333, 'g': 0.3333, 'b': 1, 'a': 0},
    ],
  },
  'objects': {
    'mother': {
      'subobj': [],
      'x': 0,
      'y': 0,
      'angle': 0,
      'opacity': 1,
      'props': {
        't': 0,
      },
      'init': function() {},
      'time': function(t, e, scene, gt) {
        if (scene === 0) {
          script.video.scale = 2. + (-1. * t);
          this.angle += 10. * e;
        } else if (scene === 1) {
          this.angle -= 1. * e;
        } else if (scene === 2) {
          script.video.scale = 1. - expf(t, 1000);
          this.opacity = 1. * t;
        } else if (scene === 3) {
          this.opacity = 0;
        }
        // var total_frames = 8. * 25.;
        // var current_frame = total_frames * gt;
        // var vibe = triangular_wave(current_frame, 1., 1.0);
        // script.video.scale = (vibe * 2.00);
      },
      'on': {
        'next_frame': function() {
          for (let i = 0; i < 10; i++) {
            let radius = 300;
            let angle = rand() * Math.PI * 2;
            let x = Math.cos(angle) * radius;
            let y = Math.sin(angle) * radius;

            angle = rand() * Math.PI * 2;
            x2 = Math.cos(angle) * radius;
            y2 = Math.sin(angle) * radius;

            let r = rand();
            let grad = r < 1 / 3. ? 'red' : r < 2 / 3. ? 'green' : 'blue';
            this.subobj.push(this.spawn({
              'id': 'test_line',
              'x': x,
              'y': y,
              'x2': x2,
              'y2': y2,
              'radiussize': 1. + rand() * 1,
              'gradient': grad,
              'props': {}
            }));
          }
        }
      },
    },
    'test_line': {
      'type': 'line',
      'gradient': 'white',
      'x': 0,
      'y': 0,
      'x2': 0,
      'y2': 0,
      'radius': 0.,
      'radiussize': 1.,
      'opacity': 1.0,
      'blending_type': blending_type.add,
      'angle': 0,
      'props': {},
      'scale': 1.0,
      'init': function() {},
      'time': function(time, elapsed) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1920,
    //'scale': 2.,
    'scale': 1.,
    'granularity': 1,
    'grain_for_opacity': true,
    'motion_blur': true,
    'min_intermediates': 20.,
    'max_intermediates': 20.,
    'update_positions': true,
    'fast_ff': true,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'width': 1920,
    'height': 1920,
    'max_intermediates': 1.,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 5.0,
      'objects': [
        {
          'id': 'mother',
          'x': 0.,
          'y': 0,
          'props': {
            'scale': false,
          }
        },
      ],
    },
    {
      'name': 'scene2',
      'duration': 1.0,
      'objects': [],
    },
    {
      'name': 'scene3',
      'duration': 1.0,
      'objects': [],
    },
    {
      'name': 'scene4',
      'duration': 1.0,
      'objects': [],
    }
  ]
};
