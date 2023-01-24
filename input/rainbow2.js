_ = {
  'gradients': {
    'red': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.5, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'circle': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 0.0,
      'gradient': 'red',
      'opacity': 1.,
      'hue': 0.,
      'rotate': 0.,
      'blending_type': blending_type.phoenix,
      'props': {'spawned': false},
      'init': function() {},
      'time': function(t, elapsed, s) {
        this.radiussize += elapsed * 100;
        // this.opacity -= elapsed / 10;
        // if (this.opacity < 0) this.opacity = 0;
        if (this.radius !== 1 && this.radiussize > 200 * Math.random()) {
          for (let n = 3, i = 0; i < n; i++) {
            let angle = ((360 / n) * i) - 45;
            let vel = angled_velocity(angle);
            this.spawn({
              'id': 'circle',
              'x': this.x + vel[0] * 100,
              'y': this.y + vel[1] * 100,
              'z': 0,
              'velocity': 10,
              'vel_x': vel[0],
              'vel_y': vel[1],
              'props': {},
              'hue': this.hue + 33 /*, 'rotate': this.rotate + 33*/
            });
            this.radius = 1;  // use as flag, currently this.props is unreliable
            this.destroy();
          }
        }
      },
    },
  },
  'video': {
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'max_intermediates': 10,
  },
  'scenes': [
    {
      'name': 'scene',
      'duration': 10.0,
      'objects': [
        {'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
  ]
};
