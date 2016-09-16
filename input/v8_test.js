/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * V8 Proof of concept with classes
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = false;
var scale         = 2;
var canvas_w      = 1920;
var canvas_h      = 1080;

var grad = new gradient();

function initialize() {
    grad.add(0.0, new color(1, 0, 0, 0));
    grad.add(0.1, new color(0, 0, 1, 0));
    grad.add(1.0, new color(0, 0, 0, 0));
}

function next() {
    var clr = grad.get(current_frame / (max_frames + 0.0));

    var tmp = new gradient();
    tmp.add(0.0, new color(clr.r, clr.g, clr.b, clr.a))
    tmp.add(1.0, new color(0, 0, 0, 0));

    add_circle(new circle(new pos(0, 0, 0), 350, 50, tmp)); 
    add_circle(new circle(new pos(0, 0, 0), current_frame, current_frame * 2., grad)); 

    write_frame();
}

