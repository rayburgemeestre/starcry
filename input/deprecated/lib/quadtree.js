/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class point {
  constructor(shape, userdata) {
    this.shape = shape;
    this.userdata = userdata;
  }
}

class quadtree {
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

    let nw = new rectangle(new pos(x + w, y, 0), w, h, new gradient());
    let ne = new rectangle(new pos(x + w, y + h, 0), w, h, new gradient());
    let se = new rectangle(new pos(x, y + h, 0), w, h, new gradient());
    let sw = new rectangle(new pos(x, y, 0), w, h, new gradient());

    this.northwest = new quadtree(nw, this.capacity);
    this.northeast = new quadtree(ne, this.capacity);
    this.southeast = new quadtree(se, this.capacity);
    this.southwest = new quadtree(sw, this.capacity);

    this.divided = true;
  }

  insert(point) {
    if (!this.boundary.contains(point.shape)) {
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
        this.northeast.insert(point) || this.northwest.insert(point) || this.southeast.insert(point) ||
        this.southwest.insert(point));
  }

  query(range, found) {
    if (!found) {
      found = [];
    }
    if (!range.intersects(this.boundary)) {
      return found;
    }
    for (let p of this.points) {
      if (range.contains(p.shape)) {
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
