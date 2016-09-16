/**
 * Generate video where circle radius increases by time.
 */
var stdin         = true;
var fps           = 25;
var canvas_w      = 3840; // 4K UHD
var canvas_h      = 2160;
var scale         = 1;

function input(line) {
    // define a nice gradient
    var tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1)) // red
    tmp.add(0.7, new color(0, 0, 1, 1)); // blue

    // draw a huge circle with it as a background
    add_circle(new circle(new pos(0, 0, 0), 0, 5000 /*size*/, tmp));

    // draw textline from stdin
    add_text(0, 0, 0, 100, line, 'center');

    // write these shapes to a frame
    write_frame();
}

