/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class projection
{
    constructor(canvas_w, canvas_h) {
        this.w = canvas_w;
        this.h = canvas_h;
        this.half_w = canvas_w / 2;
        this.half_h = canvas_h / 2;
    }
    random_position() {
        return this.translate(new vector2d(
            rand() * this.w,
            rand() * this.h
        ));
    }
    translate(position) {
        // translate from centered (0,0 == middle of canvas) to top left oriented (0,0 == top left).
        return new vector2d(
            position.x - this.half_w,
            position.y - this.half_h
        );
    }
    wrap_position(position)
    {
        while (position.x < -this.half_w) position.x += this.w;
        while (position.y < -this.half_h) position.y += this.h;
        while (position.x > this.half_w) position.x -= this.w;
        while (position.y > this.half_h) position.y -= this.h;
        return new vector2d(position.x, position.y);
    }
}
