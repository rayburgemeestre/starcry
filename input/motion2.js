_ = {
  'gradients': {
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 0, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    'yellow': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 0, 'a': 0},
    ],
    'red_bg': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
    'white_2': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.8, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
    'black': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 0, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'black_2': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.8, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.9, 'r': 0, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      'width': 1920,
      'height': 1080,
    }
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
        const stepsize = 50;
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
            'id': i < 25 ? 'red_ball' : 'ball',
            'x': x,
            'y': y,
            'z': 0,
            'vel_x': 0,
            'vel_y': 0,
            'props': {'grad': i === 0 || i === 3 ? 'red' : 'black'}
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
            let angle = get_angle(obj.x, obj.y, 0, 0);
            var rads = angle * Math.PI / 180.;
            var move = 0.5;
            var new_x = (Math.cos(rads) * move);
            var new_y = (Math.sin(rads) * move);
            obj.velocity = obj.x === 0 && obj.y === 0 ? 0 : 0.5;
            obj.vel_x = new_x;
            obj.vel_y = new_y;
          }
        }
        if (t > 0.75) {
          /*
          var ss = 0.25 / 25.0;
          for (let obj of this.subobj) {
            if (obj.vel_x > ss) obj.vel_x -= ss;
            if (obj.vel_x < -ss) obj.vel_x += ss;
            if (obj.vel_y > ss) obj.vel_y -= ss;
            if (obj.vel_y < -ss) obj.vel_y += ss;
          }
          */
        }
        if (t > 0 && t < 0.7) {
          for (let obj of this.subobj) {
            if (obj.radiussize < 20) obj.radiussize += 5 * e;
          }
        }
        // TODO: manipulating sub object gradients does not propagate
        // either via gradients or props
      },
    },
    'ball': {
      'type': 'circle',
      'collision_group': 'cg1',
      'toroidal': 't1',
      'blending_type': blending_type.normal,
      'gradients': [
        [1.0, 'black'],
        [0.0, 'black_2'],
      ],
      'radius': 0,
      'radiussize': 0.0,
      'props': {'grad': 'black', 'grad1': 1.0, 'grad2': 0.0},
      'init': function() {
        this.gradients[0][1] = this.props.grad;
      },
      'time': function(t) {
        if (t > 0.5) {
          let q = (t - 0.5) / 0.1;
          if (q > 1.0) q = 1.0;
          this.gradients[0][0] = 1.0 - q;
          this.gradients[1][0] = q;
        }
        this.velocity = t * 100;
        if (t >= 0.7) this.velocity = (1.0 - (t - 0.6 /*to stop earlier*/) / 0.3) * 100;
        if (this.velocity < 0) this.velocity = 0;
      },
    },
    'red_ball': {
      'type': 'circle',
      'collision_group': 'cg1',
      'toroidal': 't1',
      'blending_type': blending_type.normal,
      'gradients': [
        [1.0, 'red'],
      ],
      'radius': 0,
      'radiussize': 5.0,
      'props': {'grad': 'black', 'grad1': 1.0, 'grad2': 0.0},
      'init': function() {},
      'time': function(t, elapsed) {
        this.velocity = t * 100;
        if (t >= 0.7) this.velocity = (1.0 - (t - 0.6 /*to stop earlier*/) / 0.3) * 100;
        if (this.velocity < 0) this.velocity = 0;
      },
    },
    'bg': {
      'type': 'circle',
      'gradients': [
        [1.0, 'green'],
        [0.0, 'yellow'],
      ],
      'blending_type': blending_type.normal,
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 2500,
      'init': function() {},
      'time': function(t, elapsed) {
        this.gradients[0][0] = 1.0 - t;
        this.gradients[1][0] = t;
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
    // TODO: grain moet ergens anders toegevoegd gaan worden, tijdens dubbel bufferen??
    'grain_for_opacity': true,
    'dithering': true,
    // TODO#2: avoid harsch transitions from movement
    //  (problem is that steps won't be updated.)
    // 'min_intermediates': 5,
    'minimize_steps_per_object': true,
    'bg_color': {'r': 1., 'g': 1., 'b': 1., 'a': 1},
  },
  'preview': {
    'motion_blur': false,
    'min_intermediates': 2,
    'max_intermediates': 2,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 1920,
    'height': 1080,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
