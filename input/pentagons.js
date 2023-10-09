_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      'width': 1920,
      'height': 1080,
    }
  },
  'objects': {
    'triangles': {
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        let n = 5;
        let subobj = [];
        for (let i = 0; i < n; i++)
          subobj.push(this.spawn({
            'id': 'ball',
            'x': 0,
            'y': 0,
            'z': 0,
            'velocity': rand() * 100.,
            'vel_x': ((rand() * 2.) - 1.),
            'vel_y': ((rand() * 2.) - 1.),
          }));
        for (let i = 0; i < n; i++) {
          for (let j = 0; j < n; j++) {
            if (i < j) {
              let obj1 = subobj[i];
              let obj2 = subobj[j];
              this.spawn3(
                  {
                    'id': 'line',
                    'label': this.random_hash,
                    'x': obj1.x,
                    'y': obj1.y,
                    'x2': obj2.x,
                    'y2': obj2.y,
                    'z': 0,
                  },
                  obj1,
                  obj2);
            }
          }
        }
      },
      'time': function(t) {
        // let n = 5;
        // let index = 0;
        //
        // output('A');
        // for (let i = 0; i < n; i++) {
        //   for (let j = 0; j < n; j++) {
        //     if (i < j) {
        //       let obj1 = this.subobj[i];
        //       let obj2 = this.subobj[j];
        //       output('A');
        //       while (this.subobj[index].id != 'line') index++;
        //       this.subobj[index].x = obj1.x;
        //       this.subobj[index].y = obj1.y;
        //       this.subobj[index].x2 = obj2.x;
        //       this.subobj[index].y2 = obj2.y;
        //       if (!Array.isArray(obj1.props.left)) obj1.props.left = [];
        //       if (!Array.isArray(obj2.props.left)) obj2.props.left = [];
        //       if (!Array.isArray(obj1.props.right)) obj1.props.right = [];
        //       if (!Array.isArray(obj2.props.right)) obj2.props.right = [];
        //
        //       let add_obj1 = true;
        //       let add_obj2 = true;
        //
        //       for (let o of obj1.props.left) {
        //         if (o.unique_id == this.subobj[index].unique_id) {
        //           output('ADDING cnflict: ' + o.unique_id)
        //           add_obj1 = false;
        //           break;
        //         }
        //       }
        //       for (let o of obj2.props.right) {
        //         if (o.unique_id == this.subobj[index].unique_id) {
        //           output('ADDING cnflict2: ' + o.unique_id)
        //           add_obj2 = false;
        //           break;
        //         }
        //       }
        //
        //       if (add_obj1) {
        //         output('ADDING TO OBJ 1');
        //         obj1.props.left.push(this.subobj[index]);
        //       }
        //       if (add_obj2) {
        //         output('ADDING TO OBJ 2');
        //         obj2.props.right.push(this.subobj[index]);
        //       }
        //       index++;
        //     }
        //   }
        // }
        this.angle = 360. * t;
      },
    },
    'ball': {
      'type': 'circle',
      'opacity': 0,
      'toroidal': 't1',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 10,
      'radiussize': 10.0,
      'props': {'left': [], 'right': []},
      'init': function() {
        this.props.seed = rand();
        this.props.vel = this.velocity;
      },
      'time': function(t) {
        for (let lefty of this.props.left) {
          lefty.x = this.x;
          lefty.y = this.y;
        }
        for (let righty of this.props.left) {
          righty.x2 = this.x;
          righty.y2 = this.y;
        }
        this.velocity = this.props.vel;  // * Math.sin(t * 100 * this.props.seed);
      },
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 2,
      'init': function() {},
      'time': function(t) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 3,
    'granularity': 1,
    'grain_for_opacity': true,
    'dithering': true,
    //'max_intermediates': 2,
    'min_intermediates': 2,
    'max_intermediates': 2,
    'minimize_steps_per_object': true,  // this guy is interesting to debug!!
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'motion_blur': false,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 1920,
    'height': 1080,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 10,
    'objects': [
      {'id': 'triangles', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 3., 'scale': 1., 'props': {}},
    ],
  }]
};
