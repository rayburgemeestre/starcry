_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.3, 'r': 1, 'g': 1, 'b': 1, 'a': 0.0},
      {'position': 0.6, 'r': 1, 'g': 1, 'b': 1, 'a': 0.8},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'textures': {},
  'objects': {
    'obj': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 0,
      'radiussize': 5.0,
      'opacity': 1.0,
      'props': {'radius': 200., 'opacity': 1.0, 'flag': false},
      'init': function() {},
      'velocity': 0.,
      'time': function(t, elapsed) {
        function squared(num) {
          return num * num;
        }
        function squared_dist(num, num2) {
          return (num - num2) * (num - num2);
        }
        function get_distance(x, y, x2, y2) {
          return sqrt(squared_dist(x, x2) + squared_dist(y, y2));
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
          var angle = atan(slope);
          if (dx < 0) angle += Math.PI;  // M_PI;

          angle = 180.0 * angle / Math.PI;  // M_PI;

          while (angle < 0.0) angle += 360.0;

          return angle;
        }

        this.radius += elapsed * 40;
        if (this.radius >= this.props.radius) {
          this.radius = this.props.radius;
        }
        if (this.radius >= this.props.radius && this.subobj.length == 0) {
          var n = 3.;
          for (var i = 0; i < n; i++) {
            var angle = ((360 / n) * i) + 30;
            var rads = angle * Math.PI / 180.0;
            var ratio = 1.0;
            var move = this.radius * ratio * -1;
            var new_x = 0 - (Math.cos(rads) * move);
            var new_y = 0 - (Math.sin(rads) * move);

            var new_radius = 0.6180339887498547 * this.props.radius;
            // var new_radius = 1. * this.props.radius;
            if (new_radius >= 5.) {
              output('new recursive');
              this.subobj.push({
                'id': 'obj',
                'label': 'sub1',
                'x': new_x,
                'y': new_y,
                'z': 0,
                'opacity': (0.6180339887498547 * this.props.opacity),
                'props': {'radius': new_radius}
              });
            }
          }
        }
        // if (this.radius >= this.props.radius) {
        //   this.opacity -= elapsed/10.;
        //   if (this.opacity <= 0.) {
        //     this.opacity = 0.;
        //   }
        // }
        if (t > 0.5 && !this.props.flag) {
          this.props.flag = true;
          let [x, y] = random_velocity();
          this.vel_x = x;
          this.vel_y = y;
          this.velocity = 10.;
          output('OK');
        }
      },
    },
    'mgr': {
      'type': '',
      'radius': 0,
      'radiussize': 0,
      'init': function() {},
      'time': function(t, elapsed) {
        script.video.scale = expf(t, 20) * 5. + 0.25;
      },
    },
  },
  'video': {
    'duration': 30,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.3,  // 0.25,
    'rand_seed': 1,
    'granularity': 1,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'obj', 'label': 'first', 'x': 0, 'y': 125, 'z': 0, 'props': {}},
      // {'id': 'mgr', 'label': 'mgr', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
