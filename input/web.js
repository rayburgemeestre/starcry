_ = {
  'gradients': {
    'red': '#ff0000',
    'white': '#ffffff',
  },
  'objects': {
    'line_of_dots': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 0.0,
      'gradient': 'red',
      'opacity': 1.,
      'hue': 0.,
      'rotate': 0.,
      'blending_type': blending_type.phoenix,
      'props': {'spawned': false},
      'init': function() {
        if (this.level > 0) return;

        for (let n = 5, a = 0; a < n; a++) {
          let the_angle = 360 / n * a - (360 / n) + 90;
          let the_angle2 = 360 / n * (a + 1) - (360 / n) + 90;

          let left = [];
          let right = [];
          let dots = 20;
          for (let i = 0; i < dots; i++) {
            let x = 0;
            let y = i * -30;
            var angle = the_angle;

            var rads = angle * Math.PI / 180.0;
            var move = y;
            var new_x = 0 + (Math.cos(rads) * move);
            var new_y = 0 + (Math.sin(rads) * move);

            left.push(this.spawn({'id': 'dot', 'x': new_x, 'y': new_y, 'z': 0, 'props': {}}));

            var angle = the_angle2;

            var rads = angle * Math.PI / 180.0;
            var move = y;
            var new_x = 0 + (Math.cos(rads) * move);
            var new_y = 0 + (Math.sin(rads) * move);

            right.push(this.spawn({'id': 'dot', 'x': new_x, 'y': new_y, 'z': 0, 'props': {}}));
          }
          for (let i = 0; i < dots; i++) {
            let a = left[dots - 1 - i];
            let b = right[i];
            let line = this.spawn3({'id': 'glowing_line'}, a, b);
            let line2 = this.spawn3({'id': 'line'}, a, b);
          }
        }
      },
      'time': function(t, elapsed, s) {
        this.rotate += elapsed * 10;
        this.hue += elapsed * 10;
      },
    },
    'dot': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 10.0,
      'gradient': 'red',
      'opacity': 0.,
      'hue': 0.,
      'rotate': 0.,
      'blending_type': blending_type.phoenix,
      'props': {'spawned': false},
      'init': function() {},
      'time': function(t, elapsed, s) {},
    },
    'circle': {
      'type': 'circle',
      'radius': 100,
      'radiussize': 1.0,
      'gradient': 'red',
      'opacity': 1.,
      'hue': 0.,
      'rotate': 0.,
      'blending_type': blending_type.normal,
      'props': {'spawned': false},
      'init': function() {},
      'time': function(t, elapsed, s) {},
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 1,
      'init': function() {
        // TODO: figure out a way that we can inherit the cascade stuff...
        // let line = this.spawn3({'id': 'line'}, a, b);
        // for now lets do it all manually
        // yeah, this isn't going to work well..
        // let line = this.spawn({'id': 'glowing_line', 'x': this.x, 'y': this.y, 'x2': this.x2, 'y2': this.y2,
        // 'props': {}});
      },
      'time': function(t, e, s, tt) {

      },
    },
    'glowing_line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 20,
      'opacity': 0.1,
      'init': function() {
        // how can we reference the parent.
      },
      'time': function(t, e, s, tt) {},
    },
  },
  // 'video': {
  //   'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  //   'max_intermediates': 10,
  // },
  // 'scenes': [
  //   {
  //     'name': 'scene',
  //     'duration': 10.0,
  //     'objects': [
  //       {'id': 'line_of_dots', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
  //     ]
  //   },
  // ]
  'video': {
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'max_intermediates': 10,
    'scale': 1.00,
  },
  'scenes': [
    {
      'name': 'scene',
      'duration': 10.0,
      'objects': [
        // {'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'radius': 10, 'props': {}},
        // {'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'line_of_dots', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
  ]
};
