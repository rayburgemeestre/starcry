/**
 * Generate video where circle radius increases by time.
 */
var fps           = 25;
// 4K UHD
var canvas_w      = 3840;
var canvas_h      = 2160;
var scale         = 1;

function input(line_) {
    //output(line_);
    
    // level 1
    set_background_color(new color(line_ % 255, 0, 255, 1));
    
    // level 2
    var tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1))
    tmp.add(0.7, new color(0, 0, 1, 1));
    add_circle(new circle(new pos(0, 0, 0), 0, 5000, tmp));

    add_text(0, 0, 0, 150, 'This is frame ' + line_, 'center');

    write_frame();
}

function next() {
}

