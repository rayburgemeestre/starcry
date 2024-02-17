_ = {
  'gradients': {
    'red': '#ff0000@0.9',
  },
  'objects': {
    'circle': {
      'type': 'circle',
      'gradient': 'red',
      'radius': 0.0,
      'radiussize': 50.0,
      'init': function() {
        [this.vel_x, this.vel_y] = random_velocity();
        this.velocity = rand() * 30;
        // this is not spawn depth.. we need something like that...

        return;
        this.props.depth = this.props.depth || 1;
        if (this.props.depth < 10) {
          output('ret = ' + this.spawn_parent({'id': 'circle', 'props': {'depth': this.props.depth + 1}}));
        }
      },
      'time': function(t, e) {
        this.radius += e * 10;
        if (Math.abs(this.x) > 1000 || Math.abs(this.y) > 1000) {
          this.destroy();
          this.spawn_parent({'id': 'circle', 'props': {'depth': this.props.depth + 1}});
        }
        // waarom dit dan weer crasht...
        if (rand() < 0.01) {
          this.spawn({'id': 'circle', 'props': {'depth': this.props.depth + 1}});
        }
      }
    },
  },
  'video': {
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'max_intermediates': 1,
    /*
  'sample': {
    'include': 0.1,  // include one second.
    'exclude': 4.9,  // then skip 5 seconds, and so on.
  },
  */
  },
  'scenes': [
    {'name': 'scene', 'duration': 60.0, 'objects': [{'id': 'circle'}]},
  ]
};
