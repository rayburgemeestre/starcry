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
        // grow circle size until max is reached
        this.radius += elapsed * 40;
        if (this.radius >= this.props.radius) {
          this.radius = this.props.radius;
        }

        // if max is reached spawn self recursively with smaller max radius
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
            // continue recursion if radius exceeds 5 only
            if (new_radius >= 5.) {
              this.subobj.push(this.spawn({
                'id': 'obj',
                'label': 'sub1',
                'x': new_x,
                'y': new_y,
                'z': 0,
                'opacity': (0.6180339887498547 * this.props.opacity),
                'props': {'radius': new_radius}
              }));
            }
          }
        }

        // half-way through the video give each circle random direction and velocity
        if (t > 0.5 && !this.props.flag) {
          this.props.flag = true;
          let [x, y] = random_velocity();
          this.vel_x = x;
          this.vel_y = y;
          this.velocity = 10.;
        }
      },
    },
  },
  'video': {
    'duration': 30,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.3,
    'rand_seed': 1,
    'granularity': 1,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'obj', 'label': 'first', 'x': 0, 'y': 125, 'z': 0, 'props': {}},
    ],
  }]
};
