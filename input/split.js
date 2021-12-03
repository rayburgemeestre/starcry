_ = {
  'gradients': {
    'full_white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
    ],
    'white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.5, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'splitter': {
      'init': function() {
        this.subobj.push({'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {'x': 0, 'y': 0}});
        this.subobj.push({'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {'x': 0, 'y': 0}});
      },
      'time': function(t, e, scene, tt) {
        let jump = 100;
        if (scene === 0) {
          this.subobj[0].props.x = expf(t, 50) * jump;
          this.subobj[1].props.x = expf(t, 50) * -jump;
          this.subobj[0].angle = 360 * t;
          this.subobj[1].angle = 360 * t;
        } else if (scene === 1) {
          this.subobj[0].props.x = jump;
          this.subobj[1].props.x = -jump;
          this.subobj[0].angle = 360 * t;
          this.subobj[1].angle = 360 * t;
        } else {
          this.subobj[0].props.x = jump;
          this.subobj[1].props.x = -jump;
        }
        script.video.scale = (expf(tt, 10000) * 3.0) + 1.5;
      },
    },
    'text': {
      'type': 'text',
      'gradient': 'full_white',
      'x': 0,
      'y': 0,
      'text': 'Yeahh',
      'text_size': 60,
      'text_align': 'center',
      'init': function() {},
      'time': function(t, e, s) {
        this.text = script.scenes[s].name;
        this.text_size = t * 100;
      },
    },
    'bg': {
      'type': 'circle',
      // contains a bug, debug!
      //  'texture': 'clouds1',
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 1000.,
      'angle': 0.,
      'init': function() {},
      'time': function() {},
    },
    'circle': {
      'type': 'circle',
      'gradient': 'white',
      'x': -50,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 50.0,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, scene) {
        let jump = 100;
        var total_frames = 1.0 /*scene duration*/ * 30.;
        var current_frame = total_frames * t;
        switch (scene) {
          case 0:
            this.x = this.props.x + triangular_wave(current_frame, 1., 1.0) * (t * jump);
            break;
          case 1:
            this.x = this.props.x + triangular_wave(current_frame, 1., 1.0) * ((1.0 - t) * jump);
            break;
          case 2:
            this.x = this.props.x;
            break;
        }
      },
    },
  },
  'video': {
    'fps': 30,
    'width': 1080,
    'height': 1080,
    'scale': 1,
    'rand_seed': 5,
    'granularity': 1,
    'min_intermediates': 5.,
    'max_intermediates': 50.,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 0},
    'grain_for_opacity': true,
  },
  'preview': {
    'max_intermediates': 1.,
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 0.5 * 5,
      'objects': [
        // {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'splitter', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'text', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
    {'name': 'scene2', 'duration': 0.05 * 5, 'objects': []},
    {'name': 'scene3', 'duration': 0.5 * 5, 'objects': []},
  ]
};