/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * Generate video where circle radius increases by time.
 */
var fps                     = 25;
var frames_per_blendingmode = 3 /* seconds */ * fps;
var max_frames              = 29 /* blending modes */ * frames_per_blendingmode;
var canvas_w = 1920;
var canvas_h = 1080;
var scale = 1;

var blending_modes = [
    [ 'normal', blending_type.normal ],
    [ 'add', blending_type.add ],
    [ 'lighten', blending_type.lighten ],
    [ 'darken', blending_type.darken ],
    [ 'multiply', blending_type.multiply ],
    [ 'average', blending_type.average ],
    [ 'add', blending_type.add ],
    [ 'subtract', blending_type.subtract ],
    [ 'difference', blending_type.difference ],
    [ 'negation', blending_type.negation ],
    [ 'screen', blending_type.screen ],
    [ 'exclusion', blending_type.exclusion ],
    [ 'overlay', blending_type.overlay ],
    [ 'softlight', blending_type.softlight ],
    [ 'hardlight', blending_type.hardlight ],
    [ 'colordodge', blending_type.colordodge ],
    [ 'colorburn', blending_type.colorburn ],
    [ 'lineardodge', blending_type.lineardodge ],
    [ 'linearburn', blending_type.linearburn ],
    [ 'linearlight', blending_type.linearlight ],
    [ 'vividlight', blending_type.vividlight ],
    [ 'pinlight', blending_type.pinlight ],
    [ 'hardmix', blending_type.hardmix ],
    [ 'reflect', blending_type.reflect ],
    [ 'glow', blending_type.glow ],
    [ 'phoenix', blending_type.phoenix ],
    [ 'hue', blending_type.hue ],
    [ 'saturation', blending_type.saturation ],
    [ 'color', blending_type.color ],
    [ 'luminosity', blending_type.luminosity ]
];

function next() {
    //var diff = +new Date() - begin;
    //seconds = (diff / 1000.0) % 60;
    //set_background_color(color(0.5, 0.5, 0.5, 0));
    seconds = current_frame;
    var tmp = new gradient();
    tmp.add(0.0,   new color(1, 0, 0, 1));
    tmp.add(0.333, new color(0, 1, 0, 1));
    tmp.add(0.666, new color(0, 0, 1, 1));
    tmp.add(1.0,   new color(1, 1, 0, 0));

    var progress = (current_frame / max_frames) * blending_modes.length;
    var index = Math.floor(progress);
    progress -= index;

    var test = new line(new pos(- canvas_w, 0, 0), new pos(canvas_w, 0, 0), canvas_h/4, tmp);
    add_line(test);

    if (index < blending_modes.length) {
        add_text(0, 10 - (canvas_h / 2), 0, 24, 'current_frame = ' + index + ' progress = ' + progress, 'center');

        var bm = blending_modes[index][0];
        add_text(0, 100 - (canvas_h / 2), 0, 50, 'blending mode ' + bm, 'center');
        blendingtype = blending_modes[index][1];
        var test = new circle(new pos(200, 0, 0), 10 * (10 * progress), 50.0, tmp);
        test.blending_type = blendingtype;
        add_circle(test);
        var test2 = new circle(new pos(-200, 0, 0), 10 * (10 * progress), 50.0, tmp);
        test2.blending_type = blending_type.normal;
        add_circle(test2);
    }

    if (current_frame == max_frames) {
        close();
    }
}
