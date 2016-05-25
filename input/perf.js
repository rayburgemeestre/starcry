/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * Generate video where circle radius increases by time.
 */
var fps           = 25;
var max_frames    = 9999999; // seconds
var realtime      = true;
//var seconds       = 0;
//var begin         = +new Date();
var canvas_w      = 1920;
var canvas_h      = 1080;
var canvas_w      = 3840;
var canvas_h      = 2160;
var scale         = 1;

function next() {
    add_circle(new circle(new pos(0, 0, 0), current_frame % 100, 5.0, new color(1, 1, 1, 0)));
    write_frame();
}
