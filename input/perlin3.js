_ = {
  'gradients': {
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 1.0},
    ],
    'green': [
      {'position': 0.0, 'r': 0, 'g': 1, 'b': 0, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 1.0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 1.0},
    ],
    'white2': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': .5},
      {'position': 0.7, 'r': 1, 'g': 1, 'b': 1, 'a': .5},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': .5},
    ],
  },
  'textures': {
    'clouds1': {
      'type': 'perlin',
      'size': 3000.,
      'octaves': 7,
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
      //'texture': 'clouds1',
      'radius': 0,
      'radiussize': 1920,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {},
    },
    'rings': {
      'radius': 0,
      'radiussize': 0,
      'props': {'depth': 30},
      'subobj': [],
      'x': 0,
      'y': 150,
      'angle': -25,
      'init': function() {
        var x = this.props.depth / 50.;
        var step = expf(x, 300) * 100.;
        this.subobj.push({'id': 'ring', 'x': -300, 'y': 150, 'x2': 300, 'y2': 150, 'scale': step, 'z': 0, 'props': {}});
        this.subobj.push({'id': 'ring', 'x': -300, 'y': 150, 'x2': 0, 'y2': -300, 'scale': step, 'z': 0, 'props': {}});
        this.subobj.push({'id': 'ring', 'x': 0, 'y': -300, 'x2': 300, 'y2': 150, 'scale': step, 'z': 0, 'props': {}});
        if (this.props.depth > 0) {
          this.subobj.push({
            'id': 'rings',
            'x': 0,
            'y': 0,
            'x2': 0,
            'y2': 0,
            'z': 0,
            'angle': 5,
            'props': {'depth': this.props.depth - 1}
          });
        }
      },
      'time': function(t, elapsed) {},
    },
    'ring': {
      'type': 'line',
      'gradients': [[0.1, 'white2']],
      'radiussize': 20,
      'props': {},
      'opacity': 0.5,
      'init': function() {},
      'time': function(t, elapsed) {
        // this.radiussize = t * 600.;
        // this.radiussize = 20. - (logn(t, 100) * 15.);
        this.radiussize = 20. - (logn(t, 10000) * 15.);
        this.gradients[0][0] = t;
        // hack test
        this.gradients[0][0] = 1.;
        // this.scale = 1.0 + (logn(t, 100) * 1);
        script.video.scale = 1.0 + (logn(t, 100) * 1);
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
    'scale': 1.,
    'rand_seed': 1,
    'granularity': 1000,
    'update_positions': false,
    // 'sample': {
    //   'include': 1.,  // include one second.
    //   'exclude': 5.,  // then skip 5 seconds, and so on.
    // },
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      // {
      //   'id': 'obj',
      //   'x': 0,
      //   'y': 0,
      //   'z': 0,
      //   'radius': 0,
      //   'radiussize': 0,
      //   'texture': 'clouds1',
      //   'gradient': 'red',
      //   'seed': 1,
      //   'props': {}
      // },
      // {
      //   'id': 'obj',
      //   'x': 0,
      //   'y': 0,
      //   'z': 0,
      //   'radius': 0,
      //   'radiussize': 0,
      //   'texture': 'clouds2',
      //   'gradient': 'red',
      //   'seed': 2,
      //   'props': {}
      // },
      {
        'id': 'obj',
        'x': 0,
        'y': 0,
        'z': 0,
        'radius': 0,
        'texture': 'clouds1',
        'gradient': 'blue',
        'seed': 3,
        'props': {}
      },
      {'id': 'rings', 'x': 0, 'y': 75, 'z': 0, 'radius': 0, 'radiussize': 0, 'props': {}},
    ],
  }]
};
