// The following settings are mandatory
const stdin         = true; // make starcry read from standard input (stdin)
const fps           = 25;   // number of frames per second for the resulting video
const canvas_w      = 480;  // width
const canvas_h      = 320;  // height

// We are going to store the last 20 lines of stdin
let buffer_of_lines = [];

// The following function is invoked for each line received from standard input
function input(line_of_text)
{
    // buffer this line
    buffer_of_lines.push(line_of_text);

    // keep only the last 20 lines
    buffer_of_lines = buffer_of_lines.slice(-20);

    // generate a frame that displays all lines from the buffer
    for (let i=1; i<=buffer_of_lines.length; i++) {
        // start placing text from buffer at the very top Y position
        let text_y_offset = canvas_h / 2.0;
        // every line will be placed 15 pixels underneath the previous line
        text_y_offset -= i * 15;
        // render text centered
        add_text(0, 0 - text_y_offset, 0, 10, buffer_of_lines[i - 1], 'center');
    }
    write_frame();
}
