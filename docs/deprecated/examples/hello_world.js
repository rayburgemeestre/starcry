// The following settings are mandatory
const fps           = 25;       // number of frames per second for the resulting video
const max_frames    = 10 * fps; // total number of frames to render in order to get 10
                                //   seconds worth of video.
const canvas_w      = 480;      // width
const canvas_h      = 320;      // height
const realtime      = true;

// The following function is invoked before rendering each frame
function next() {
    // We want to draw a circle in the middle of the video canvas
    const center = new pos(0, 0, 0);
    // The current_frame constiable is automatically incremented w/each frame
    const radius = current_frame;
    // The thickness in pixels for the given circle
    const radius_size = 5.0;
    // Color of the circle is specified in the form of a gradient
    const red_gradient = new gradient();
    // The gradient will start with red and gradually transform into transparent
    red_gradient.add(0.0, new color(1, 0, 0, 1)); // red
    red_gradient.add(1.0, new color(0, 0, 0, 0)); // transparent

    // Construct a circle object and add it to the system for rendering
    add_circle(new circle(center, radius, radius_size, red_gradient));
}
