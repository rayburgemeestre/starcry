_ = {
  'gradients': {
    'redbg': [
      {'position': 0.0, 'r': 1., 'g': 0., 'b': 0., 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.3, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.6, 'r': 1, 'g': 1, 'b': 1, 'a': 0.8},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'textures': {
    'clouds1': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 7,
      'persistence': 0.45,
      'percentage': 1.0,
      'scale': 100.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 5.,
    },
    'clouds2': {
      'type': 'fractal',
      'size': 3000.,
      'octaves': 2,
      'persistence': 0.45,
      'percentage': 0.4,
      'scale': 10.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 10.,
    },
  },
  'objects': {
    'balls': {
      'x': 0,
      'y': 0,
      'props': {'direction': 0, 'steps': 1, 'step': 1, 'times': 0, 'start': false},
      'subobj': [],
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        const stepsize = 70;
        let directions = [
          [0, -stepsize],  // up
          [stepsize, 0],   // right
          [0, stepsize],   // down
          [-stepsize, 0],  // left
        ];
        let x = 0;
        let y = stepsize / 2;
        // for (let i = 0; i < 210; i++) {
        for (let i = 0; i < 100; i++) {
          x += directions[this.props.direction][0];
          y += directions[this.props.direction][1];
          if (this.props.step === this.props.steps) {
            this.props.step = 0;
            this.props.times += 1;
            this.props.direction += 1;
            this.props.direction %= 4;
          }
          if (this.props.times === 2) {
            this.props.steps += 1;
            this.props.times = 0;
            this.props.step += 1;
          }
          this.props.step++;

          this.subobj.push(this.spawn({
            'id': 'ball',
            'label': 'ball#' + i,
            'x': x,
            'y': y,
            'z': 0,
            'vel_x': 0,
            'vel_y': 0,
            'props': {'grad': i === 0 || i === 3 ? 'red' : 'white'}
          }));
        }
        for (let obj of this.subobj) {
          let [x, y] = random_velocity();
          obj.vel_x = x;
          obj.vel_y = y;
          obj.velocity = 50;
        }
      },
      'time': function(t, e) {},
    },
    'ball': {
      'collision_group': 'group1',
      'type': 'circle',
      'gradients': [
        [1.0, 'white'],
        [0.0, 'red'],
      ],
      'radius': 0,
      'radiussize': 20.0,
      'props': {'grad': 'white'},
      'init': function() {},
      'time': function(t, elapsed) {
        // while (this.x + (1920 / 2) < 0) this.x += 1920;
        // while (this.y + (1080 / 2) < 0) this.y += 1080;
        // while (this.x + (1920 / 2) > 1920) this.x -= 1920;
        // while (this.y + (1080 / 2) > 1080) this.y -= 1080;
      },
      'subobj': [],
      'on': {
        'collide': function(other) {
          this.subobj.push(
              this.spawn({'id': 'tempring', 'label': 'ring_' + this.label, 'x': 0, 'y': 0, 'z': 0, 'props': {}}));
        }
      }
    },
    'tempring': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 0,
      'radiussize': 5.0,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {
        this.radius += elapsed * 50;
        if (this.radius >= 200.) {
          this.exists = false;
        }
      },
    },
    'bg': {
      'type': 'circle',
      'gradient': 'redbg',
      'texture': 'clouds1',
      'radius': 0,
      'radiussize': 0,
      'init': function() {
        this.seed = rand();
      },
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    //    'width': 14043,
    //    'height': 9933,
    'scale': 1.,
    //    'scale': 7.3,
    'rand_seed': 1,
    'granularity': 1,
    'grain_for_opacity': false,
    'dithering': true,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 5,
    'objects': [
      // {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
