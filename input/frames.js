var fps           = 25;
var canvas_w      = 1920/4;
var canvas_h      = 1080/4;
var max_frames    = 10 * fps;

function next(line_of_text)
{
    set_background_color(new color(current_frame/250.0, current_frame/250.0, current_frame/250.0, 1));
    add_text(0, 0, 0, 100, '' + current_frame, 'center');
}
