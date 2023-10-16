_ = {
  'gradients': {
    'red': '#770000',
    'white': '#ffffff',
    'two': '#ff0000',
  },
  'objects': {
    'mother': {
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
        this.props.offset = this.level;

        // this.scale = this.props.scale; // workaround?
        let this_level = this.level - this.props.offset;
        if (this_level > 0) return;

        let [vel_x, vel_y] = random_velocity1(this_level + this.props.id);
        vel_x *= 50 * this.scale;
        vel_y *= 50 * this.scale;
        let rand_angle = rand() * 20;

        for (let deg = 0; deg < 360; deg += 360 / 5) {
          o = this.spawn({
            'id': 'dot',
            'x': vel_x,
            'y': vel_y,
            'z': 0,
            'props': {
              'vel_x': vel_x,
              'vel_y': vel_y,
              'ra': rand_angle,
              'mother': this.unique_id,
              'offset': this.props.offset
            },
            'rotate': deg,
            'scale': this.scale
          });
          this.spawn2({'id': 'line'}, o);
          this.spawn2({'id': 'line2'}, o);
          o = this.spawn({
            'id': 'dot',
            'x': vel_x,
            'y': vel_y,
            'z': 0,
            'props': {
              'vel_x': vel_x,
              'vel_y': vel_y,
              'ra': -rand_angle,
              'mother': this.unique_id,
              'offset': this.props.offset
            },
            'rotate': deg,
            'scale': this.scale
          });
          this.spawn2({'id': 'line'}, o);
          this.spawn2({'id': 'line2'}, o);
        }
      },
      'time': function(t, elapsed, s) {},
    },
    'dot': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 2.0,
      'gradient': 'red',
      'opacity': 0.5,
      'hue': 0.,
      'rotate': 0.,
      'blending_type': blending_type.add,
      'props': {'spawned': false},
      'init': function() {
        let this_level = this.level - this.props.offset;  // j;//this.attr('level') + 1;
        if (this_level >= 20) return;

        let [vx, vy] = random_velocity1(this_level);
        this.vel_x = vx;
        this.vel_y = vy;
        this.velocity = 1;

        let [vel_x, vel_y] = random_velocity1(this_level);
        vel_x *= 10;
        vel_y *= 10;
        // this.props.vel_x = vel_x;
        // this.props.vel_y = vel_y;
        let rand_angle = (rand1(this_level) * 360) - 180;
        // output("random for: " + this_level + " + " + rand_angle);
        this.props.ra = rand_angle;

        let o = this.spawn({
          'id': 'dot',
          'x': this.props.vel_x,
          'y': this.props.vel_y,
          'z': 0,
          'rotate': this.props.ra,
          'scale': this.scale,
          'props': this.props
        });
        this.spawn2({'id': 'line'}, o);
        this.spawn2({'id': 'line2'}, o);

        // output("debug: " + this.props.mother + " ?");
        //  this.spawn3({'id': 'line'}, o, this.props.mother);
      },
      'time': function(t, elapsed, s) {
        let this_level = this.level - this.props.offset;  // j;//this.attr('level') + 1;
        if (rand1(this_level + t * 250) < 0.1) {
          let [vel_x, vel_y] = random_velocity1(this_level + t * 250);
          vel_x *= 50 * this.scale;
          vel_y *= 50 * this.scale;
          this.vel_x = vel_x;
          this.vel_y = vel_y;
        }
      },
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
      'gradient': 'red',
      'radius': 0,
      'opacity': 0.5,
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
    'line2': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'two',
      'radius': 0,
      'opacity': 0.1,
      'radiussize': 1,
      'init': function() {},
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
    'min_intermediates': 10,
    'max_intermediates': 10,
    'rand_seed': 40,
    // 'scale': 3.00,
    // 'width': 10000,
    // 'height': 10000,
  },
  'preview': {
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'min_intermediates': 1,
    'max_intermediates': 1,
  },
  'scenes': [
    {
      'name': 'scene',
      'duration': 10,
      'objects': [
        {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'scale': 1, 'props': {'id': 1}},
        {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'scale': 1, 'props': {'id': 2}},
        {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'scale': 1, 'props': {'id': 3}},
        {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'scale': 1, 'props': {'id': 4}},
        {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'scale': 1, 'props': {'id': 5}},
      ]
    },
  ]
};
