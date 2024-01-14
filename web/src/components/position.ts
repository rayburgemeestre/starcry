/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

export type original_x = number;
export type original_y = number;
export type transformed_x = number;
export type transformed_y = number;

export class position {
  center_x: original_x;
  center_y: original_x;
  offset_x: original_x;
  offset_y: original_x;
  canvas_w: original_x;
  canvas_h: original_x;
  scale: number;
  original_x: original_x;
  original_y: original_y;

  constructor(
    center_x: original_x,
    center_y: original_x,
    offset_x: original_x,
    offset_y: original_x,
    canvas_w: original_x,
    canvas_h: original_x,
    scale: number,
    x: original_x,
    y: original_x
  ) {
    this.center_x = center_x;
    this.center_y = center_y;
    this.offset_x = offset_x;
    this.offset_y = offset_y;
    this.canvas_w = canvas_w;
    this.canvas_h = canvas_h;
    this.scale = scale;
    this.original_x = x;
    this.original_y = y;
  }

  x(): transformed_x {
    return (this.original_x - this.center_x) * this.scale - this.offset_x + this.canvas_w / 2;
  }

  y(): transformed_y {
    return (this.original_y - this.center_y) * this.scale - this.offset_y + this.canvas_h / 2;
  }
}
