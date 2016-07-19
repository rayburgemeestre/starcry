/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var fps           = 25;
var max_frames    = 30 * fps; // 30 seconds
//
// 4K UHD resolution
var canvas_w      = 3840;
var canvas_h      = 2160;

function next() {
    var tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1))
    tmp.add(0.7, new color(0, 0, 1, 1));
    add_circle(new circle(new pos(0, 0, 0), 0, 3000, tmp));
    write_frame();
}
