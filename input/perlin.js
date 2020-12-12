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
      'percentage': 1.0,
      'scale': 100.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 1.0,
      'speed': 10.,
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
        this.radiussize = logn(t, 100) * 1200.;
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
        for (var i = 0; i < 5; i++) {
          // this.subobj.push({'id': 'ring', 'x': 0, 'y': 0, 'radius': i * 100, 'z': 0, 'blending_type': 7, 'props': {
          // }});
          this.subobj.push(
              {'id': 'ring', 'x': -300, 'y': 150, 'x2': 300, 'y2': 150, 'radius': i * 100, 'z': 0, 'props': {}});
          this.subobj.push(
              {'id': 'ring', 'x': -300, 'y': 150, 'x2': 0, 'y2': -300, 'radius': i * 100, 'z': 0, 'props': {}});
          this.subobj.push(
              {'id': 'ring', 'x': 0, 'y': -300, 'x2': 300, 'y2': 150, 'radius': i * 100, 'z': 0, 'props': {}});
        }
      },
      'time': function(t, elapsed) {
        // this.radiussize = t * 600.;
      },
    },
    'ring': {
      'type': 'line',
      'gradients': [[0.1, 'white2']],
      // 'texture': 'clouds2',
      'radiussize': 20,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {
        // this.radiussize = t * 600.;
        // this.radiussize = 20. - (logn(t, 100) * 15.);
        this.radiussize = 20. - (logn(t, 10000) * 15.);
        this.gradients[0][0] = t;
        // this.scale = 1.0 + (logn(t, 100) * 1);
        script.video.scale = 1.0 + (logn(t, 100) * 1);
      },
      'blending_type': blending_type.add,
      'scale': 1.0,
    },
  },
  'video': {
    'duration': 20,
    'fps': 30,
    'width': 1920,
    'height': 1080,
    'scale': 1.,
    'rand_seed': 1,
    'granularity': 10,
    'experimental_feature1': false,
    'sample': {
      'include': 1.,  // include one second.
      'exclude': 5.,  // then skip 5 seconds, and so on.
    },
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {
        'id': 'obj',
        'x': -100,
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
        'x': +100,
        'y': 75,
        'z': 0,
        'radius': 0,
        'radiussize': 0,
        'texture': 'clouds1',
        'gradient': 'blue',
        'seed': 3,
        'props': {}
      },
      // {'id': 'obj', 'x': -300, 'y': 150 + 75, 'z': 0, 'radius': 0, 'radiussize': 0, 'texture': 'clouds1', 'gradient'
      // : 'red', 'seed': 2, 'props': { }},
      // {'id': 'obj', 'x': 300, 'y': 150 + 75, 'z': 0, 'radius': 0, 'radiussize': 0, 'texture': 'clouds1', 'gradient' :
      // 'green', 'seed': 20,  'props': { }},
      // {'id': 'obj', 'x': 0, 'y': -300 + 75, 'z': 0, 'radius': 0, 'radiussize': 0, 'texture': 'clouds1', 'gradient' :
      // 'blue', 'seed': 3, 'props': { }},
      {'id': 'rings', 'x': 0, 'y': 75, 'z': 0, 'radius': 0, 'radiussize': 0, 'props': {}},
    ],
  }]
};
