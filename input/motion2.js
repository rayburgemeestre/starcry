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
    'red_bg': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
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
    ],
    'white_2': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.8, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
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
            'id': i < 30 ? 'red_ball' : 'ball',
            'x': x,
            'y': y,
            'z': 0,
            'vel_x': 0,
            'vel_y': 0,
            'props': {'grad': i === 0 || i === 3 ? 'red' : 'white'}
          });
        }
      },
      'time': function(t, e) {
        // script.video.scale += 0.1 * e;
        if (!this.props.start && t > 0.1) {
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
            obj.velocity = 50;
            obj.vel_x = velocity.x;
            obj.vel_y = velocity.y;
          }
        }
        if (t > 0.75) {
          var ss = 0.25 / 25.0;
          for (let obj of this.subobj) {
            if (obj.vel_x > ss) obj.vel_x -= ss;
            if (obj.vel_x < -ss) obj.vel_x += ss;
            if (obj.vel_y > ss) obj.vel_y -= ss;
            if (obj.vel_y < -ss) obj.vel_y += ss;
          }
        }
        if (t > 0) {
          for (let obj of this.subobj) {
            if (obj.radiussize < 20) obj.radiussize += 5 * e;
          }
        }
        if (t > 0.7) {
          for (let obj of this.subobj) {
            obj.radiussize += 20 * e;
          }
        }
        // TODO: manipulating sub object gradients does not propagate
        // either via gradients or props
      },
    },
    'ball': {
      'type': 'circle',
      'collision_group': 'cg1',
      'gradients': [
        [1.0, 'white'],
        [0.0, 'white_2'],
      ],
      'radius': 0,
      'radiussize': 0.0,
      'props': {'grad': 'white', 'grad1': 1.0, 'grad2': 0.0},
      'init': function() {
        this.gradients[0][1] = this.props.grad;
      },
      'time': function(t, elapsed) {
        if (t > 0.5) {
          let q = (t - 0.5) / 0.1;
          if (q > 1.0) q = 1.0;
          this.gradients[0][0] = 1.0 - q;
          this.gradients[1][0] = q;
        }
        if (t >= 0.5) return;
        while (this.x + (1920 / 2) < 0) this.x += 1920;
        while (this.y + (1080 / 2) < 0) this.y += 1080;
        while (this.x + (1920 / 2) > 1920) this.x -= 1920;
        while (this.y + (1080 / 2) > 1080) this.y -= 1080;
      },
    },
    'red_ball': {
      'type': 'circle',
      'collision_group': 'cg2',
      'gradients': [
        [1.0, 'red'],
      ],
      'radius': 0,
      'radiussize': 0.0,
      'props': {'grad': 'white', 'grad1': 1.0, 'grad2': 0.0},
      'init': function() {},
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
      'radiussize': 2500,
      'init': function() {},
      'time': function(t, elapsed) {
        // this.gradients[0][0] = 1.0 - t;
        // this.gradients[1][0] = t;
      },
    },
    'bg2': {
      'type': 'circle',
      'gradients': [
        [1.0, 'yellow'],
        [0.0, 'red'],
      ],
      'radius': 0,
      'radiussize': 2500,
      'init': function() {},
      'time': function(t, elapsed) {
        // this.x = (-1920/2.0) - (1920 * t);
        this.gradients[0][0] = 1.0 - t;
        this.gradients[1][0] = t;
      },
    },
  },
  'video': {
    'duration': 10,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 3,
    'granularity': 1,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      // {'id': 'bg2', 'x': -1920 / 2.0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
