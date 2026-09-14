# Emit/Absorb Effects System Specification

## Overview

The emit/absorb effects system enables objects to interact with each other through various types of emissions and absorptions. Objects can emit effects based on proximity, age, mass, or custom criteria, while other objects can absorb these effects to modify their properties dynamically.

## Core Concepts

### Emitters
Objects that broadcast effects to their surroundings. Each emitter has:
- A named effect (e.g., 'prox', 'age', 'gravity')
- A type that determines how the effect is calculated
- A maximum range or value
- An output range for normalization

### Absorbers
Objects that receive and react to emitted effects. Each absorber has:
- A reference to which emission it absorbs
- Optional range remapping
- An effect function that modifies the object's properties

## Emit Configuration

Emit effects are defined in an object's `emit` property as a key-value map:

```javascript
emit: {
  effect_name: {
    type: 'proximity|age|mass',
    max: number,              // Maximum value (pixels for proximity, seconds for age)
    range_start: number,      // Output range start (default: 0.0)
    range_end: number         // Output range end (default: 1.0)
  }
}
```

### Effect Types

#### 1. Proximity (`type: 'proximity'`)
Emits based on distance to other objects.
- `max`: Maximum distance in pixels
- Intensity: 1.0 at distance 0, 0.0 at max distance
- Example: Objects glow when near each other

#### 2. Age (`type: 'age'`)
Emits based on object lifetime.
- `max`: Maximum age in seconds
- Intensity: 0.0 at creation, 1.0 at max age
- Example: Objects fade in over time

#### 3. Mass (`type: 'mass'`)
Emits gravitational-like effects based on object mass.
- `max`: Maximum effect distance in pixels
- Intensity: Calculated based on mass and distance
- Example: Heavy objects attract lighter ones

> **Open question: does `type: 'mass'` earn its place?** Starcry already has a
> first-class gravity system (`mass` as an object property, `gravity_group`,
> `interactor::handle_gravity`) used by `input/orbit.js`, `input/spiral-gravity.js`
> and `input/collision.js`. The overlap is in the *input signal* (mass × distance),
> not in capability: built-in gravity hardcodes the response as a velocity change,
> whereas a mass emitter yields a 0..1 intensity handed to a user-written `effect`,
> which can do anything (custom force curves, damping, bounds, colour, size).
> Decide before implementing: either drop the type, or keep it and share the
> distance/mass computation with `handle_gravity` instead of writing it twice.

## Absorb Configuration

Absorb effects are defined in an object's `absorb` property:

```javascript
absorb: {
  absorber_name: {
    input: 'effect_name',     // Name of the effect to absorb
    range_start: number,      // Optional input range remapping (default: 0.0)
    range_end: number,        // Optional input range remapping (default: 1.0)
    effect: function(intensity, emitter) {
      // Modify this object based on intensity (0.0 to 1.0)
      // Optional emitter parameter provides source object reference
    }
  }
}
```

## Effect Calculation

### Proximity Calculation
```
distance = sqrt((emitter.x - absorber.x)² + (emitter.y - absorber.y)²)
raw_intensity = 1.0 - (distance / max)
intensity = clamp(raw_intensity, 0.0, 1.0)
output = range_start + (intensity * (range_end - range_start))
```

### Age Calculation
```
age = current_time - creation_time
raw_intensity = age / max
intensity = clamp(raw_intensity, 0.0, 1.0)
output = range_start + (intensity * (range_end - range_start))
```

### Mass Calculation
```
distance = sqrt((emitter.x - absorber.x)² + (emitter.y - absorber.y)²)
if (distance < max) {
  force = (emitter.mass * absorber.mass) / (distance²)
  raw_intensity = force / max_force
  intensity = clamp(raw_intensity, 0.0, 1.0)
  output = range_start + (intensity * (range_end - range_start))
}
```

## Complete Example

```javascript
// Emitter object that glows and attracts
glowing_attractor: {
  init: function() {
    this.mass = 5.0;
    this.creation_time = scene.time;
  },
  emit: {
    glow: {
      type: 'proximity',
      max: 100,
      range_start: 0.0,
      range_end: 1.0
    },
    pull: {
      type: 'mass',
      max: 200,
      range_start: 0.0,
      range_end: 0.5
    },
    maturity: {
      type: 'age',
      max: 3,
      range_start: 0.0,
      range_end: 1.0
    }
  }
},

// Absorber object that reacts to emissions
reactive_particle: {
  init: function() {
    this.velocity_x = 0;
    this.velocity_y = 0;
    this.glow = 0;
    this.opacity = 0;
  },
  time: function(t, e, scene) {
    // Apply physics
    this.x += this.velocity_x;
    this.y += this.velocity_y;
    this.velocity_x *= 0.98; // Damping
    this.velocity_y *= 0.98;
  },
  absorb: {
    brightness: {
      input: 'glow',
      effect: function(intensity) {
        this.glow = intensity;
        this.radiussize = 20 + (intensity * 10);
      }
    },
    attraction: {
      input: 'pull',
      range_start: 0.0,
      range_end: 0.1, // Scale down the mass effect
      effect: function(intensity, emitter) {
        if (emitter) {
          var dx = emitter.x - this.x;
          var dy = emitter.y - this.y;
          var dist = Math.sqrt(dx * dx + dy * dy);
          if (dist > 0) {
            this.velocity_x += (dx / dist) * intensity;
            this.velocity_y += (dy / dist) * intensity;
          }
        }
      }
    },
    fade_in: {
      input: 'maturity',
      effect: function(intensity) {
        this.opacity = intensity;
      }
    }
  }
}
```

## Best Practices

1. **Performance**: Limit the number of emit/absorb pairs for better performance
2. **Range Normalization**: Use range_start/range_end to fine-tune effect strengths
3. **Effect Functions**: Keep effect functions simple and focused on property modification
4. **Debugging**: Use output() to log intensity values during development

## Advanced Usage

### Custom Effect Types
While the system provides built-in types (proximity, age, mass), you can extend it with custom types by implementing the calculation logic in the engine.

### Chaining Effects
Objects can both emit and absorb, creating chains of interactions:
- Object A emits proximity → Object B absorbs and emits mass → Object C absorbs mass

### Conditional Effects
Effect functions can include conditions:
```javascript
effect: function(intensity) {
  if (intensity > 0.5) {
    this.state = 'activated';
    this.color = 'red';
  }
}
```

## Implementation Notes

- Effects are calculated each frame for all object pairs
- Distance calculations use Euclidean distance
- Intensity values are always normalized to 0.0-1.0 before being passed to effect functions
- The optional `emitter` parameter in effect functions provides access to the emitting object's properties