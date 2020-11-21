_ = {
  'gradients': {
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
      {'position': 0.7, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 0.8},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
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
            'props': {'grad': i === 0 || i === 3 ? 'red' : 'white'}
          });
        }

        //
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
          velocity.x *= 150;
          velocity.y *= 150;
          obj.vel_x = velocity.x;
          output('set: vel: ' + obj.vel_x);
          obj.vel_y = velocity.y;
        }
      },
      'time': function(t, e) {
        for (let obj of this.subobj) {
          obj.radiussize += 25 * e;
        }
      }
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
      'time': function(t, elapsed) {},
    },
    'bg': {
      'type': 'circle',
      'gradient': 'blue',
      'radius': 0,
      'radiussize': 2500,
      'init': function() {},
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'duration': 20,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 1,
    'granularity': 5,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
