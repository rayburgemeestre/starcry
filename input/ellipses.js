_ = {
  'gradients': {},
  'objects': {
    'main': {
      'init': function() {
        const stepsize = 200;
        let directions = [[0, -stepsize], [stepsize, 0], [0, stepsize], [-stepsize, 0]];
        let x = 0;
        let y = stepsize / 2;
        let direction = 0;
        let times = 0;
        let step = 1;
        let steps = 1;
        for (let i = 0; i < 100; i++) {
          x += directions[direction][0];
          y += directions[direction][1];
          if (step === steps) {
            step = 0;
            times += 1;
            direction += 1;
            direction %= 4;
          }
          if (times === 2) {
            steps += 1;
            times = 0;
            step += 1;
          }
          step++;
          let l = rand() * 80 + 20.;
          let s = rand() * 80 + 20.;
          if (s > l) {
            let tmp = l;
            l = s;
            s = tmp;
          }
          this.spawn({
            'id': 'ellipse',
            'x': x,
            'y': y,
            'z': 0,
            'vel_x': 0,
            'vel_y': 0,
            'attrs': {
              'longest_diameter': l,
              'shortest_diameter': s,
              'radiussize': rand() * 19 + 1.,
              'rotate': rand() * 360.,
            }
          });
        }
      }
    },
    'ellipse': {
      'type': 'script',
      'file': 'input/ellipse.js',
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 0.5,
    'rand_seed': 5,
    'granularity': 1,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'width': 512,
    'height': 512,
    'scale': 0.5,
    'max_granularity': 1,
    'max_intermediates': 1,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 5.0,
      'objects': [
        {'id': 'main', 'x': 0, 'y': 0, 'z': 0, 'scale': 1.},
      ],
    },
  ]
};
