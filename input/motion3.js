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
      {'position': 0.3, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.6, 'r': 1, 'g': 1, 'b': 1, 'a': 0.8},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'balls': {
      'x': 0,
      'y': 0,
      'props': {'direction': 0, 'steps': 1, 'step': 1, 'times': 0, 'start': false},
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
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

          let velocity = new vector2d(rand(), 0);
          velocity.rotate(rand() * 360);
          this.spawn({
            'id': 'ball',
            'x': x,
            'y': y,
            'z': 0,
            'vel_x': velocity.x,
            'vel_y': velocity.y,
            'velocity': 1.,
            'props': {'grad': i === 0 || i === 3 ? 'red' : 'white'}
          });
        }
      },
      'time': function(t, e) {},
    },
    'explosion': {
      'type': 'circle',
      'gradient': 'blue',
      'velocity': 1.0,
      'vel_x': 0.0,
      'vel_y': 0.0,
      'x': 0.0,
      'y': 0.0,
      'radius': 0,
      'radiussize': 5,
      'init': function() {},
      'time': function(t, elapsed) {
        // if (this.props.enabled) {
        this.radius += elapsed * 3;
        //} else {
        //  this.radius -= elapsed * 3;
        // }
        if (this.radius > 15) {
          this.radius = 15;
        }
        if (this.radius < 0) {
          this.radius = 0;
        }
      },
      'props': {}
    },
    'ball': {
      'collision_group': 'group1',
      'type': 'circle',
      'gradients': [
        [1.0, 'white'],
        [0.0, 'red'],
      ],
      'radius': 0,
      'radiussize': 5.0,
      'props': {'grad': 'white'},
      'init': function() {
        // this.gradient = this.props.grad;
        // this.spawn({'id': 'explosion', 'x': 0, 'y': 0, 'z': 0, 'vel_x': 0, 'vel_y': 0, 'props': {}});
      },
      'time': function(t, elapsed) {
        this.velocity = 10 * t;

        const steps = elapsed / 5;  // tie this to elapsed time
        if (this.gradients[1][0] > steps) {
          this.gradients[0][0] += steps;
          this.gradients[1][0] -= steps;
        } else {
          // TODO: find alternative for this...
          // if (this.subobj.length) {
          //   this.subobj[0].props.enabled = false;
          // }
        }
        while (this.x + (1920 / 2) < 0) this.x += 1920;
        while (this.y + (1080 / 2) < 0) this.y += 1080;
        while (this.x + (1920 / 2) > 1920) this.x -= 1920;
        while (this.y + (1080 / 2) > 1080) this.y -= 1080;
      },
      'collide': function(other) {
        this.opacity = 0.;
        this.spawn({'id': 'explosion', 'x': 0, 'y': 0, 'z': 0, 'vel_x': 0, 'vel_y': 0, 'props': {}});

        this.gradients[0][0] = 0.0;
        this.gradients[1][0] = 1.0;

        // TODO: find alternative...
        // if (this.subobj.length) {
        //   this.subobj[0].props.enabled = true;
        // }
      }
    },
    'bg': {
      'type': 'circle',
      'gradient': 'blue',
      'radius': 0,
      'radiussize': 5,
      'init': function() {},
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 1,
    'granularity': 1,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 20,
    'objects': [
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
