_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1.0},
      {'position': 0.5, 'r': 1, 'g': 0, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'yellow': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 0, 'a': 1.0},
      {'position': 0.5, 'r': 1, 'g': 1, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 0, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 0, 'g': 1, 'b': 0, 'a': 1.0},
      {'position': 0.5, 'r': 0, 'g': 1, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    'msx_blue': [
      // 5900/5500/E000/ff00
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0.5},
      {'position': 0.2, 'r': 0x59 / 255., 'g': 0x55 / 255., 'b': 0xe0 / 255., 'a': 1.0},
      {'position': 1.0, 'r': 0x59 / 255., 'g': 0x55 / 255., 'b': 0xe0 / 255., 'a': 1.0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0.5},
      {'position': 0.4, 'r': 0, 'g': 0, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1.0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1.0, 'b': 1., 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 1.0, 'b': 1., 'a': 0.0},
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
        // var x = 0.90 * (5000/1920.);
        var x = 0.90;
        // was 5.
        script.video.scale = x - expf(t, 1.);
      },
    },
    'bg': {
      'type': 'circle',
      'gradient': 'blue',
      'texture': 'clouds1',
      'radius': 0,
      'radiussize': 1920,
      'opacity': 1.0,
      'props': {},
      'init': function() {},
      'velocity': 0.,
      'time': function(t, elapsed) {},
      // 'motion_blur': false,
    },
    'line': {
      'type': 'line',
      'gradient': 'red',
      'radiussize': 4.0,
      'opacity': 1.0,
      'props': {},
      'blending_type': blending_type.pinlight,
      'init': function() {},
      'time': function(t, elapsed) {},
    },
    'obj': {
      'type': 'circle',
      'gradient': 'blue',
      //'radius': 200,
      'radius': 200,
      'radiussize': 5.0,
      'opacity': 1.0,
      'props': {},
      'blending_type': blending_type.pinlight,
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

        var real = [];
        real.push([this.x, this.y]);

        for (var i = 0, n = 6.; i < n; i++) {
          var angle = ((360 / n) * i) + 30;
          var rads = angle * Math.PI / 180.0;
          var ratio = 1.0;
          var move = this.props.radius * 2. * ratio * -1;
          var new_x = 0 + (Math.cos(rads) * move);
          var new_y = 0 + (Math.sin(rads) * move);

          real.push([new_x, new_y]);
          this.subobj.push(this.spawn({
            'id': 'obj',
            'label': 'sub1#' + this.subobj.length,
            'x': new_x,
            'y': new_y,
            'z': 0,
            'props': {'child': true, 'radius': this.props.radius}
          }));

          move = this.props.radius * 4. * ratio * -1;
          new_x = 0 + (Math.cos(rads) * move);
          new_y = 0 + (Math.sin(rads) * move);
          key = Math.round(new_x) + '' + Math.round(new_y);
          real.push([new_x, new_y]);
          this.subobj.push(this.spawn({
            'id': 'obj',
            'label': 'sub1b#' + this.subobj.length,
            'x': new_x,
            'y': new_y,
            'z': 0,
            'props': {'child': true, 'radius': this.props.radius}
          }));
        }

        for (let i = 0; i < real.length; i++) {
          for (let j = 0; j < real.length; j++) {
            if (i >= j) continue;
            var a = real[i];
            var b = real[j];
            // var dist = get_distance(a[0], a[1], b[0], b[1]);
            var dist = get_distance(0, 0, a[0], a[1]);
            var dist2 = get_distance(0, 0, b[0], b[1]);
            this.subobj.push(this.spawn({
              'id': 'line',
              'label': 'lineX',
              'x': a[0],
              'y': a[1],
              'x2': b[0],
              'y2': b[1],
              'z': 0,
              'gradient': dist < (this.props.radius * 3.1) && dist2 < (this.props.radius * 3.1) ? 'yellow' : 'red',
              'props': {}
            }));
          }
        }
        // encapsulating circles
        this.subobj.push(this.spawn({
          'id': 'obj',
          'label': 'e#' + this.subobj.length,
          'x': 0,
          'y': 0,
          'z': 0,
          'radius': this.props.radius * 3,
          'gradient': 'green',
          'props': {'child': true, 'radius': this.props.radius}
        }));
        this.subobj.push(this.spawn({
          'id': 'obj',
          'label': 'e#' + this.subobj.length,
          'x': 0,
          'y': 0,
          'z': 0,
          'radius': this.props.radius * 4,
          'gradient': 'green',
          'props': {'child': true, 'radius': this.props.radius}
        }));
        this.subobj.push(this.spawn({
          'id': 'obj',
          'label': 'e#' + this.subobj.length,
          'x': 0,
          'y': 0,
          'z': 0,
          'radius': this.props.radius * 5,
          'gradient': 'green',
          'props': {'child': true, 'radius': this.props.radius}
        }));
      },
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 0.70,
    //        'width': 5000,
    //       'height': 5000,
    //      'scale': 0.70 * (5000/1920.),
    'rand_seed': 1,
    'granularity': 1.,
    'grain_for_opacity': true,
    'extra_grain': 0.2,
    'motion_blur': true,
    'perlin_noise': true,
    'dithering': true,
    'update_positions': true,
    'max_intermediates': 15,
    // 'sample': {
    //   'include': 1.,  // include one second.
    //   'exclude': 5.,  // then skip 5 seconds, and so on.
    // },
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 1,
    'objects': [
      {'id': 'a', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'obj', 'x': 0, 'label': 'whaa', 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
