script = {
  "gradients": {
    "red": [
      {
        "position": 0,
        "r": 1,
        "g": 0,
        "b": 0,
        "a": 1
      },
      {
        "position": 1,
        "r": 1,
        "g": 0,
        "b": 0,
        "a": 0
      }
    ],
    "green": [
      {
        "position": 0,
        "r": 0,
        "g": 1,
        "b": 0,
        "a": 1
      },
      {
        "position": 1,
        "r": 0,
        "g": 1,
        "b": 0,
        "a": 0
      }
    ],
    "blue": [
      {
        "position": 0,
        "r": 0,
        "g": 0,
        "b": 1,
        "a": 1
      },
      {
        "position": 1,
        "r": 0,
        "g": 0,
        "b": 1,
        "a": 0
      }
    ]
  },
  "objects": {
    "obj0": {
      "x": 0,
      "y": 0,
      "props": {},
      "subobj": [],
      "radius": 0,
      "radiussize": 0,
      "angle": 0,
      "init": function() {
        this.spawn({"id": "obj1", "x": -300, "y": 0, "z": 0, "props": {}});
        this.spawn({"id": "obj1", "x": 300, "y": 0, "z": 0, "props": {}});
        this.spawn({"id": "obj1", "x": 0, "y": -300, "z": 0, "props": {}});
        this.spawn({"id": "obj1", "x": 0, "y": +300, "z": 0, "props": {}});
      },
      "time": function(t, e, scene) {
        switch (scene) {
          case 0:
            break;
          case 1:
            this.angle = -360. * expf(t, 10.);
            break;
        }
      }
    },
    "obj1": {
      "x": 0,
      "y": 0,
      "init": function() {
        this.spawn({"id": "obj2", "x": -100, "y": 0, "z": 0, "props": {"maxradius": 100}});
        this.spawn({"id": "obj2", "x": 0, "y": 0, "z": 0, "props": {"maxradius": 200}});
        this.spawn({"id": "obj2", "x": 100, "y": 0, "z": 0, "props": {"maxradius": 300}});
      }
    },
    "obj2": {
      "type": "circle",
      "radius": 100,
      "hue": 180,
      "radiussize": 10,
      "gradients": [
        [
          1,
          "red"
        ],
        [
          0,
          "green"
        ],
        [
          0,
          "blue"
        ]
      ],
      "props": {
        "maxradius": 250
      },
      "blending_type": blending_type.normal,
      "init": function() {
        this.radius = 0;
      },
      "time": function(t, elapsed, s) {
        this.hue += elapsed * 200;
        this.gradients[0][0] = 1.0 - t;
        this.gradients[2][0] = t;
        switch (s) {
          case 0:
          case 1:
            this.radius += 100.0 * elapsed;
            this.radius %= this.props.maxradius;
            break;
          case 2:
          case 3:
          case 4:
            this.radius = logn(t, 1.) * 1920.0;
        }
      }
    }
  },
  "video": {
    "fps": 25,
    "width": 1920,
    "height": 1080,
    "scale": 1,
    "rand_seed": 5,
    "granularity": 1,
    "minimize_steps_per_object": false,
    "bg_color": {
      "r": 0,
      "g": 0,
      "b": 0,
      "a": 1
    },
    "grain_for_opacity": false
  },
  "preview": {
    "width": 512,
    "height": 512
  },
  "scenes": [
    {
      "name": "scene1",
      "duration": 3,
      "objects": [
        {
          "id": "obj1",
          "x": 50,
          "y": 0,
          "z": 0,
          "props": {}
        }
      ]
    }
  ],
  "textures": {}
}
;

;

;
