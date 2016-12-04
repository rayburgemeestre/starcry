/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var fps                     = 25;
var frames_per_blendingmode = 1 /* seconds */ * fps;
var max_frames              = 29 /* blending modes */ * frames_per_blendingmode;
var canvas_w                = 1920;
var canvas_h                = 1080;
var scale                   = 1;

include("./input/blending_modes_array.js");

function next() {
    seconds = current_frame;
    var tmp = new gradient();
    tmp.add(0.0,   new color(1, 0, 0, 1));
    tmp.add(0.333, new color(0, 1, 0, 1));
    tmp.add(0.666, new color(0, 0, 1, 1));
    tmp.add(1.0,   new color(1, 1, 0, 0));

    var progress = (current_frame / max_frames) * blending_modes.length;
    var index = Math.floor(progress);
    progress -= index;

    var test = new line(new pos(- canvas_w, 0, 0), new pos(canvas_w, 0, 0), canvas_h, tmp);
    add_line(test);

    if (index < blending_modes.length) {
        var bm = blending_modes[index][0];
        add_text(0, 100 - (canvas_h / 2), 0, 50, 'blending mode ' + bm, 'center');
        blendingtype = blending_modes[index][1];
        var test = new circle(new pos(200, 0, 0), 10 * (10 * progress), 50.0, tmp);
        test.blending_type = blendingtype;
        add_circle(test);
        var test2 = new circle(new pos(-200, 0, 0), 10 * (10 * progress), 50.0, tmp);
        //test2.blending_type = blending_type.normal;
        add_circle(test2);
    }
    if (current_frame == max_frames) {
        close();
    }
}
