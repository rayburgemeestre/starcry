// The following settings are mandatory
var stdin = true;     // make starcry read from standard input (stdin)
var realtime = true;  // make starcry read from standard input (stdin)
var fps = 25;         // number of frames per second for the resulting video
var canvas_w = 1920;  // width
var canvas_h = 1080;  // height // currently a memory leak with this resolution
var max_frames = -1;
// We are going to store the last 20 lines of stdin
var buffer_of_lines = [];

// The following function is invoked for each line received from standard input
function input(line_of_text) {
  if (line_of_text.startsWith('ATOP')) buffer_of_lines = [];

  // buffer this line
  buffer_of_lines.push(line_of_text);

  // keep only the last 20 lines
  // buffer_of_lines = buffer_of_lines.slice(-1 * ((canvas_h / 15) - 1));
}

function next(line_of_text) {
  // generate a frame that displays all lines from the buffer
  for (var i = 1; i <= buffer_of_lines.length; i++) {
    // start placing text from buffer at the very top Y position
    var text_x_offset = canvas_w / 2.0;
    var text_y_offset = canvas_h / 2.0;
    // every line will be placed 15 pixels underneath the previous line
    text_y_offset -= i * 15;
    // render text centered
    add_text(0 - text_x_offset, 0 - text_y_offset, 0, 10, buffer_of_lines[i - 1], 'left');
  }
}
