var fps = 60;
var canvas_w = 1920;
var canvas_h = 1080;
var max_frames = 5 * fps;

function next(line_of_text) {
  const r = current_frame / max_frames;
  set_background_color(new color(r, r, r, 1));
  add_text(0, 0, 0, 100, '' + current_frame, 'center');
}
