/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

export class rectangle {
  constructor(x, y, width, height) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
  }

  contains(x, y) {
    return x >= this.x && x < this.x + this.width && y >= this.y && y < this.y + this.height;
  }
}

export class circle {
  constructor(x, y, radius) {
    this.x = x;
    this.y = y;
    this.radius = radius;
  }

  contains(x, y) {
    let d = Math.pow(x - this.x, 2.0) + Math.pow(y - this.y, 2.0);
    return d <= Math.pow(this.radius * 2, 2.0);
  }

  intersects(range) {
    const xDist = Math.abs(range.x - this.x);
    const yDist = Math.abs(range.y - this.y);

    // radius of the circle
    const r = this.radius;

    const w = range.width;
    const h = range.height;

    const edges = Math.pow(xDist - w, 2) + Math.pow(yDist - h, 2);

    // no intersection
    if (xDist > r + w || yDist > r + h) return false;

    // intersection within the circle
    if (xDist <= w || yDist <= h) return true;

    // intersection on the edge of the circle
    // return edges <= this.rSquared;
    return edges <= Math.pow(this.radius, 2);
  }
}

export class point {
  constructor(shape, userdata) {
    this.shape = shape;
    this.userdata = userdata;
  }
}

export class quadtree {
  constructor(boundary, capacity) {
    if (!boundary) {
      throw TypeError('boundary is null or undefined');
    }
    if (!(boundary instanceof rectangle)) {
      throw TypeError('boundary should be a rectangle');
    }
    if (typeof capacity !== 'number') {
      throw TypeError(`capacity should be a number but is a ${typeof capacity}`);
    }
    if (capacity < 1) {
      throw RangeError('capacity must be greater than 0');
    }
    this.boundary = boundary;
    this.capacity = capacity;
    this.points = [];
    this.divided = false;
  }
  subdivide() {
    let x = this.boundary.x;
    let y = this.boundary.y;
    let w = this.boundary.width / 2;
    let h = this.boundary.height / 2;

    let nw = new rectangle(x + w, y, w, h);
    let ne = new rectangle(x + w, y + h, w, h);
    let se = new rectangle(x, y + h, w, h);
    let sw = new rectangle(x, y, w, h);

    this.northwest = new quadtree(nw, this.capacity);
    this.northeast = new quadtree(ne, this.capacity);
    this.southeast = new quadtree(se, this.capacity);
    this.southwest = new quadtree(sw, this.capacity);

    this.divided = true;
  }

  insert(point) {
    if (!this.boundary.contains(point.shape.x, point.shape.y)) {
      return false;
    }
    if (this.points.length < this.capacity) {
      this.points.push(point);
      return true;
    }
    if (!this.divided) {
      this.subdivide();
    }
    return (
      this.northeast.insert(point) ||
      this.northwest.insert(point) ||
      this.southeast.insert(point) ||
      this.southwest.insert(point)
    );
  }

  query(range, found) {
    if (!found) {
      found = [];
    }
    if (!range.intersects(this.boundary)) {
      return found;
    }
    for (let p of this.points) {
      if (range.contains(p.shape.x, p.shape.y)) {
        found.push(p);
      }
    }
    if (this.divided) {
      this.northwest.query(range, found);
      this.northeast.query(range, found);
      this.southwest.query(range, found);
      this.southeast.query(range, found);
    }
    return found;
  }
}
