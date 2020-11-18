_ = {
  'gradients': {
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'yellow': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ]
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
        let y = 0;
        for (let i = 0; i < 210; i++) {
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

          this.subobj.push({
            'id': 'ball',
            'x': x,
            'y': y,
            'z': 0,
            'vel_x': 0,
            'vel_y': 0,
            'props': {'grad': i === 0 ? 'red' : 'white'}
          });
        }
        // this.subobj.push({
        //   'id': 'ball',
        //   'x': 0,
        //   'y': 0,
        //   'z': 0,
        //   'vel_x': undefined,
        //   'vel_y': undefined,
        //   'props': {'grad': i === 0 ? 'red' : 'white'}
        // });
      },
      'time': function(t, e) {
        if (!this.props.start && t > 0.05) {
          this.props.start = true;

          class vector2d {
            constructor(x = 0, y = 0) {
              this.x = x;
              this.y = y;
            }
            rotate(degrees) {
              const radian = this.degrees_to_radian(degrees);
              const sine = Math.sin(radian);
              const cosine = Math.cos(radian);
              this.x = this.x * cosine - this.y * sine;
              this.y = this.x * sine + this.y * cosine;
            }
            degrees_to_radian(degrees) {
              const pi = 3.14159265358979323846;
              return degrees * pi / 180.0;
            }
          }

          for (let obj of this.subobj) {
            let velocity = new vector2d(rand(), 0);
            velocity.rotate(rand() * 360);
            velocity.x *= 50;
            velocity.y *= 50;
            obj.vel_x = velocity.x;
            obj.vel_y = velocity.y;
          }
        }
      },
    },
    'ball': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 0,
      'radiussize': 20.0,
      'props': {'grad': 'white'},
      'init': function() {
        this.gradient = this.props.grad;
      },
      'time': function(t, elapsed) {
        while (this.x + (1920 / 2) < 0) this.x += 1920;
        while (this.y + (1080 / 2) < 0) this.y += 1080;
        while (this.x + (1920 / 2) > 1920) this.x -= 1920;
        while (this.y + (1080 / 2) > 1080) this.y -= 1080;
      },
    },
    'bg': {
      'type': 'circle',
      'gradients': [
        [1.0, 'blue'],
        [0.0, 'yellow'],
      ],
      'radius': 0,
      'radiussize': 1920,
      'init': function() {},
      'time': function(t, elapsed) {
        // this.gradients[0][0] = 1.0 - t;
        // this.gradients[1][0] = t;
      },
    },
  },
  'video': {
    'duration': 20,
    'fps': 30,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 1,
    'granularity': 1,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
