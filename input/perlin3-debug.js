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
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': .0},
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
      'radius': 10,
      'radiussize': 10,
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
      'y': 0,
      // 'angle': -25,
      'init': function() {
        var x = this.props.depth / 50.;
        var step = expf(x, 300) * 100.;
        this.subobj.push(
            this.spawn({'id': 'ring', 'x': -200, 'y': 0, 'x2': 200, 'y2': 0, 'z': 0, 'props': {'scale': step}}));
        // this.subobj.push(
        //     this.spawn({'id': 'ring', 'x': -300, 'y': 150, 'x2': 0, 'y2': -300, 'z': 0, 'props': {'scale': step}}));
        // this.subobj.push(
        //     this.spawn({'id': 'ring', 'x': 0, 'y': -300, 'x2': 300, 'y2': 150, 'z': 0, 'props': {'scale': step}}));
        // if (this.props.depth > 0) {
        //   this.subobj.push(this.spawn({
        //     'id': 'rings',
        //     'x': 0,
        //     'y': 0,
        //     'x2': 0,
        //     'y2': 0,
        //     'z': 0,
        //     // 'angle': 5 * this.props.depth,
        //     'props': {'depth': this.props.depth - 1}
        //   }));
        // }
      },
      'time': function(t, elapsed) {},
    },
    'ring': {
      'type': 'line',
      //'gradients': [[0.1, 'white2']],
      'radiussize': 20,
      'props': {},
      'opacity': 1.,
      'init': function() {},
      'time': function(t, elapsed) {
        // this.radiussize = t * 600.;
        // this.radiussize = 20. - (logn(t, 100) * 15.);
        // this.radiussize = 20. - (logn(t, 10000) * 15.);
        // this.gradients[0][0] = t;
        // // hack test
        // this.gradients[0][0] = 1.;
        // this.scale = 1.0 + (logn(t, 100) * 1);
        // script.video.scale = 1.0 + (logn(t, 100) * 1);
        // this.scale = this.props.scale
        // this.radiussize = 20 * this.props.scale;
        this.angle = 360 * t;
      },
      'blending_type': blending_type.normal,
      'scale': 1.0,
    },
  },
  'video': {
    'duration': 5,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.,
    'rand_seed': 1,
    'granularity': 10000,
    'motion_blur': false,
    'update_positions': true,
    'grain_for_opacity': true,
    // 'sample': {
    //   'include': 1.,  // include one second.
    //   'exclude': 5.,  // then skip 5 seconds, and so on.
    // },
    // 'bg_color': {'r': 1., 'g': 0., 'b': 0., 'a': 1},
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'max_intermediates': 1,
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
      // {
      //   'id': 'obj',
      //   'x': 0,
      //   'y': 0,
      //   'z': 0,
      //   'radius': 0,
      //   'texture': 'clouds1',
      //   'gradient': 'blue',
      //   'seed': 3,
      //   'props': {}
      // },
      {'id': 'obj', 'x': 0, 'y': -50, 'z': 0, 'props': {}},
      {'id': 'rings', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
