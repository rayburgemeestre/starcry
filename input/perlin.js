_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 0, 'g': 1, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
    'white2': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': .5},
      {'position': 0.7, 'r': 1, 'g': 1, 'b': 1, 'a': .5},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
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
      'speed': 10.,
    },
  },
  'objects': {
    'obj': {
      'type': 'circle',
      'gradient': 'white1',
      //'texture': 'clouds1',
      'radius': 0,
      'radiussize': 100,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {
        this.radiussize = logn(t, 10) * 600.;
      },
    },
    'rings': {
      'radius': 0,
      'radiussize': 0,
      'props': {},
      'subobj': [],
      'x': 0,
      'y': 150,
      'init': function() {
        for (var i = 0; i < 30; i++) {
          var x = i / 50.;
          var step = expf(x, 10) * 100.;
          this.subobj.push(this.spawn(
              {'id': 'ring', 'x': -300, 'y': 150, 'x2': 300, 'y2': 150, 'scale': step, 'z': 0, 'props': {}}));
          this.subobj.push(
              this.spawn({'id': 'ring', 'x': -300, 'y': 150, 'x2': 0, 'y2': -300, 'scale': step, 'z': 0, 'props': {}}));
          this.subobj.push(
              this.spawn({'id': 'ring', 'x': 0, 'y': -300, 'x2': 300, 'y2': 150, 'scale': step, 'z': 0, 'props': {}}));
        }
      },
      'time': function(t, elapsed) {},
    },
    'ring': {
      'type': 'line',
      'gradient': 'white2',
      'radiussize': 20,
      'props': {},
      'opacity': 1.,
      'init': function() {},
      'time': function(t, elapsed) {
        // this.radiussize = t * 600.;
        // this.radiussize = 20. - (logn(t, 100) * 15.);
        this.radiussize = 20. - (logn(t, 10000) * 15.);
        // this.gradients[0][0] = t;
        // this.scale = 1.0 + (logn(t, 100) * 1);
        script.video.scale = script.video.init_scale + (logn(t, 100) * 1);
      },
      'blending_type': blending_type.add,
      'scale': 1.0,
    },
  },
  'video': {
    'duration': 5,
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 0.5,
    'init_scale': 0.5,
    'rand_seed': 1,
    'granularity': 100,
    'update_positions': false,
    // 'sample': {
    //   'include': 1.,  // include one second.
    //   'exclude': 5.,  // then skip 5 seconds, and so on.
    // },
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'minimize_steps_per_object': true,
    'grain_for_opacity': true,
    // 'min_intermediates': 5,
    // 'max_intermediates': 5,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {
        'id': 'obj',
        'x': -300,
        'y': 75,
        'z': 0,
        'radius': 0,
        'radiussize': 0,
        'texture': 'clouds1',
        'gradient': 'red',
        'seed': 2,
        'props': {}
      },
      {
        'id': 'obj',
        'x': 0,
        'y': 75,
        'z': 0,
        'radius': 0,
        'radiussize': 0,
        'texture': 'clouds1',
        'gradient': 'green',
        'seed': 20,
        'props': {}
      },
      {
        'id': 'obj',
        'x': +300,
        'y': 75,
        'z': 0,
        'radius': 0,
        'radiussize': 0,
        'texture': 'clouds1',
        'gradient': 'blue',
        'seed': 3,
        'props': {}
      },
      {'id': 'rings', 'x': 0, 'y': 75, 'z': 0, 'radius': 0, 'radiussize': 0, 'props': {}},
    ],
  }]
};
