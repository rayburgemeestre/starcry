/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { original_x, original_y, position } from 'components/position';

export class draw_utils {
  center_x: number;
  center_y: number;
  offset_x: number;
  offset_y: number;
  canvas_w: number;
  canvas_h: number;
  scale: number;
  ctx: CanvasRenderingContext2D;

  constructor(
    center_x: number,
    center_y: number,
    offset_x: number,
    offset_y: number,
    canvas_w: number,
    canvas_h: number,
    scale: number,
    ctx: CanvasRenderingContext2D
  ) {
    this.center_x = center_x;
    this.center_y = center_y;
    this.offset_x = offset_x;
    this.offset_y = offset_y;
    this.canvas_w = canvas_w;
    this.canvas_h = canvas_h;
    this.scale = scale;
    this.ctx = ctx;
  }

  pos(x: original_x, y: original_y, transform = true) {
    return new position(
      this.center_x,
      this.center_y,
      this.offset_x,
      this.offset_y,
      this.canvas_w,
      this.canvas_h,
      this.scale,
      x,
      y,
      transform
    );
  }

  draw_circle(
    pos: position,
    radius: number,
    fg: string | CanvasGradient | CanvasPattern,
    bg: string | CanvasGradient | CanvasPattern
  ) {
    this.ctx.beginPath();
    this.ctx.arc(pos.x(), pos.y(), radius, 0, Math.PI * 2);
    this.ctx.strokeStyle = fg;
    this.ctx.lineWidth = 2;
    this.ctx.stroke();
    if (bg) {
      this.ctx.fillStyle = bg;
      this.ctx.fill();
    }
    this.ctx.closePath();
  }

  draw_line(pos_from: position, pos_to: position, line_width: number, fg: string | CanvasGradient | CanvasPattern) {
    this.ctx.beginPath();
    this.ctx.strokeStyle = fg;
    this.ctx.lineWidth = line_width;
    this.ctx.moveTo(pos_from.x(), pos_from.y());
    this.ctx.lineTo(pos_to.x(), pos_to.y());
    this.ctx.stroke();
    this.ctx.closePath();
  }
}
