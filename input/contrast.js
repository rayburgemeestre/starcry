_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1.0},
      {'position': 0.5, 'r': 1, 'g': 0, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'msx_blue': [
      // 5900/5500/E000/ff00
      {'position': 0.0, 'r': 0x59 / 255., 'g': 0x55 / 255., 'b': 0xe0 / 255., 'a': 1.0},
      {'position': 0.2, 'r': 0x59 / 255., 'g': 0x55 / 255., 'b': 0xe0 / 255., 'a': 1.0},
      {'position': 1.0, 'r': 0x59 / 255., 'g': 0x55 / 255., 'b': 0xe0 / 255., 'a': 1.0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': .0},
      {'position': 0.4, 'r': 0, 'g': 0, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1.0},
    ],
    'black': [
      {'position': 0.0, 'r': 0, 'g': 0., 'b': 0., 'a': 1.0},
      {'position': 0.9, 'r': 0, 'g': 0., 'b': 0., 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0., 'b': 0., 'a': 0.0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1., 'b': 1., 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 1., 'b': 1., 'a': 0.0},
    ],
  },
  'textures': {
    'clouds1': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 4,
      'persistence': 0.45,
      'percentage': 1.0,
      'scale': 25.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'clouds2': {
      'type': 'perlin',
      'size': 300.,
      'octaves': 3,
      'persistence': 0.45,
      'percentage': 0.9,
      'scale': 1.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
  },
  'objects': {
    'a': {
      'radius': 0,
      'radiussize': 0,
      'opacity': 0.,
      'props': {},
      'init': function() {},
      'velocity': 0.,
      'time': function(t, elapsed) {
        if (t >= 0) {
          script.video.scale = 1.5 + (1.0 - expf(t, 1000)) * 10.;
        }
      },
    },
    'bg': {
      'type': 'circle',
      'gradient': 'blue',
      'texture': 'clouds1',
      'radius': 0,
      'radiussize': 2000,
      'opacity': 1.0,
      'props': {},
      'init': function() {},
      'velocity': 0.,
      'time': function(t, elapsed) {},
      // 'motion_blur': false,
    },
    'bg2': {
      'seed': 3,
      'type': 'circle',
      'gradient': 'black',
      'texture': 'clouds2',
      'radius': 0,
      'radiussize': 1920,
      'opacity': 1.0,
      'props': {},
      'blending_type': blending_type.normal,
      'init': function() {},
      'velocity': 0.,
      'time': function(t, elapsed) {},
    },
    'line': {
      'type': 'line',
      'gradients': [
        [0.0, 'black'],
        [1.0, 'red'],
      ],
      'radiussize': 4.0,
      'opacity': 0.5,
      'props': {},
      'blending_type': blending_type.normal,
      'init': function() {},
      'time': function(t, elapsed, scene, global_time) {
        const k = 0.1;
        function sigmoid(z) {
          return 1 / (1 + Math.exp(-z / k));
        }
        var x = (global_time * 2) - 1.;
        global_time = sigmoid(x);
        this.gradients[0][0] = 1.0 - global_time;
        this.gradients[1][0] = global_time;
      },
    },
    'obj': {
      'type': 'circle',
      //'gradient': 'black',
      //'radius': 200,
      'angle': 0,
      'radius': 0,
      'radiussize': 0,
      'opacity': 1.0,
      'props': {},
      'blending_type': blending_type.normal,
      'velocity': 0.,
      'props': {'child': false, 'radius': 200.},
      'init': function() {
        if (this.props.child) return;
        // temp
        function squared(num) {
          return num * num;
        }
        function squared_dist(num, num2) {
          return (num - num2) * (num - num2);
        }
        function get_distance(x, y, x2, y2) {
          return Math.sqrt(squared_dist(x, x2) + squared_dist(y, y2));
        }
        function get_angle(x1, y1, x2, y2) {
          var dx = x1 - x2;
          var dy = y1 - y2;

          if (dx == 0 && dy == 0) return 0;

          if (dx == 0) {
            if (dy < 0)
              return 270;
            else
              return 90;
          }

          var slope = dy / dx;
          var angle = Math.atan(slope);
          if (dx < 0) angle += Math.PI;  // M_PI;

          angle = 180.0 * angle / Math.PI;  // M_PI;

          while (angle < 0.0) angle += 360.0;

          return angle;
        }


        var queue = [[this.x, this.y]];
        var visited = [], real = [];

        visited.push(Math.round(this.x) + '' + Math.round(this.y));
        real.push([this.x, this.y]);
        var x = 3.0;

        while (queue.length > 0) {
          var current = queue.shift();
          var n = 5.;
          // var n = 3.;

          // if dist is 3 circles apart don't recurse ?
          if (get_distance(0, 0, current[0], current[1]) >= this.props.radius * x) break;


          for (var i = 0; i < n; i++) {
            var angle = ((360 / n) * i) + 30;
            var rads = angle * Math.PI / 180.0;
            var ratio = 1.0;
            var move = this.props.radius * ratio * -1;
            var new_x = current[0] + (Math.cos(rads) * move);
            var new_y = current[1] + (Math.sin(rads) * move);

            var key = Math.round(new_x) + '' + Math.round(new_y);

            // var ratio = expf(get_distance(0, 0, new_x, new_y) / (this.radius * 3.), 10.);
            var ratio = expf(get_distance(0, 0, new_x, new_y) / (this.props.radius * x), 50.);
            // new_x -= 25. * ratio;

            if (!visited.includes(key)) {
              queue.push([new_x, new_y]);
              visited.push(key);
              real.push([new_x, new_y]);
              let [vel_x, vel_y] = random_velocity();
              this.subobj.push({
                'id': 'obj',
                'label': 'sub1#' + this.subobj.length,
                'x': new_x - (25. * ratio),
                'y': new_y,
                'vel_x': 1.,
                'vel_y': 0.,
                'velocity': 50. * ratio,
                'opacity': 1.0 * (1.0 - ratio),
                'z': 0,
                'props': {'child': true, 'radius': this.props.radius}
              });
            }
          }
        }

        // visited = [];
        for (let i = 0; i < real.length; i++) {
          for (let j = 0; j < real.length; j++) {
            if (i >= j) continue;
            var a = real[i];
            var b = real[j];
            if (get_distance(a[0], a[1], b[0], b[1]) >= 1.5 * this.props.radius) continue;
            this.subobj.push(
                {'id': 'line', 'label': 'lineX', 'x': a[0], 'y': a[1], 'x2': b[0], 'y2': b[1], 'z': 0, 'props': {}});
          }
        }
      },
      'time': function(t, elapsed, scene, global_time) {
        this.scale = 2.0 + (Math.sin(t * 10) * 2.);
        this.angle += elapsed * 100;
        // if (t >= 0 && t <= 1.0) {
        //   script.video.scale = 1.5 + (1.0 - logn(t, 10)) * 10.;
        // }
        return;
        this.radius = this.props.radius * (t * 1.02);
        // TODO: this t should not go beneath zero
        if (this.radius < 0) this.radius = 0;
        if (this.radius >= 200.) this.radius = 200.;
        // output("set: " + this.radius + " from " + this.props.radius + " * " + t);
      },
    },
  },
  'video': {
    'duration': 6,
    'fps': 25,
    'width': 1920,
    'height': 1920,
    //'scale': 11.5,
    //'scale': 0.5,
    'scale': 0.8,
    'rand_seed': 1,
    'granularity': 1.,
    'grain_for_opacity': false,
    'extra_grain': 0.,
    //'motion_blur': true,
    'perlin_noise': true,
    'dithering': true,
    'update_positions': false,
    'max_intermediates': 10,
    // 'sample': {
    //   'include': 1.,  // include one second.
    //   'exclude': 5.,  // then skip 5 seconds, and so on.
    // },
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'width': 200,
    'height': 200,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      //      {'id': 'a', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'bg2', 'x': 0, 'y': 0, 'z': 0, 'props': {}},

      {'id': 'obj', 'x': 0, 'radius': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'obj', 'x': 0, 'radius': 0, 'y': 0, 'z': 0, 'props': {}},

      //{'id': 'obj', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      //{'id': 'obj', 'x': 0, 'y': 0, 'z': 0, 'angle': 90, 'props': {}},
    ],
  }]
};
