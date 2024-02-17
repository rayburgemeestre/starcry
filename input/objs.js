_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'textures': {
    'clouds1': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 7,
      'persistence': 0.45,
      'percentage': 1.0,
      'scale': 100.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
    'clouds2': {
      'type': 'fractal',
      'size': 3000.,
      'octaves': 2,
      'persistence': 0.45,
      'percentage': 0.4,
      'scale': 10.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 1.,
    },
  },
  'objects': {
    'obj': {
      'type': 'circle',
      'gradient': 'white1',
      'texture': 'clouds2',
      'radius': 0,
      'radiussize': 100,
      'opacity': 1.00,
      'props': {},
      'init': function() {
        this.spawn({'id': 'obj2', 'x': 0, 'y': 0, 'z': 0, 'gradient': 'red', 'seed': rand() * 100, 'props': {}});
        this.spawn({'id': 'obj3', 'x': 0, 'y': 0, 'z': 0, 'gradient': 'red', 'seed': rand() * 100, 'props': {}});
        let n = 6;
        for (let i = 0; i < n; i++) {
          this.spawn({
            'id': 'obj4',
            'x': 100,
            'y': 0,
            'z': 0,
            'gradient': 'red',
            'seed': rand() * 100,
            'angle': i * 360 / n,
            'props': {}
          });
        }
      },
      'time': function(t, elapsed) {},
    },
    'obj2': {
      'type': 'circle',
      'gradient': 'white1',
      'texture': 'clouds1',
      'radius': 0,
      'radiussize': 10,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {},
    },
    'obj3': {
      'type': 'circle',
      'gradient': 'white1',
      'texture': 'clouds1',
      'radius': 0,
      'radiussize': 3,
      'props': {'spawned': false},
      'init': function() {},
      'time': function(t, elapsed) {
        let newradius = this.radius + (elapsed * 50);
        if (newradius > 200) {
          this.opacity = 0;
          if (!this.props.spawned) {
            this.props.spawned = true;
            this.spawn_parent(
                {'id': 'obj3', 'x': 0, 'y': 0, 'z': 0, 'gradient': 'red', 'seed': rand() * 100, 'props': {}});
          }
          return;
        }
        this.radius = newradius;
        // this.radius %= 200;
        this.opacity = 1. - this.radius / 200;
      },
    },
    'obj4': {
      'type': 'circle',
      'gradient': 'white1',
      'radius': 0,
      'radiussize': 0,
      'props': {},
      'init': function() {
        if (this.level < 4) {
          this.spawn({'id': 'obj', 'x': 0, 'y': 0, 'z': 0, 'gradient': 'red', 'seed': rand() * 100, 'props': {}});
        }
      },
      'time': function(t, elapsed) {
        this.angle += elapsed * 10;
      },
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 2.,
    'rand_seed': 100,
    'granularity': 100,
    'update_positions': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 30,
    'objects': [
      {'id': 'obj', 'x': 0, 'y': 0, 'z': 0, 'gradient': 'red', 'seed': rand() * 100, 'props': {}},
    ],
  }]
};
