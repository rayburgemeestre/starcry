/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * Generate video where circle radius increases by time.
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = true;
var seconds       = 0;
var begin         = +new Date();
var canvas_w = 1920;
var canvas_h = 1080;
var scale = 5;

function next() {
    //var diff = +new Date() - begin;
    //seconds = (diff / 1000.0) % 60;
    seconds = current_frame;
    var tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1))
    tmp.add(1.0, new color(0, 1, 0, 0));
    var test = new circle(new pos(0, 0, 0), seconds, 5.0, tmp);
    add_circle(test);
    write_frame();
    if (current_frame == max_frames) {
        close();
    }
}
