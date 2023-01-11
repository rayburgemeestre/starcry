[//]: # (---)
[//]: # (markdown:)
[//]: # (  image_dir: /assets)
[//]: # (  path: output.md)
[//]: # (  ignore_from_front_matter: true)
[//]: # (  absolute_image_path: false)
[//]: # (---)

# Main pipeline

[//]: # (```mermaid)
[//]: # (flowchart LR)
[//]: # (    cmd&#40;[instruction]&#41; --> starcry)
[//]: # (    starcry --> cmds[&#40;cmds&#41;])
[//]: # (    cmds --> generator)
[//]: # (    generator --> jobs[&#40;jobs&#41;])
[//]: # (    jobs --> renderer)
[//]: # (    renderer --> frames[&#40;frames&#41;])
[//]: # (    frames --> streamer)
[//]: # (    streamer --> output&#40;[output]&#41;)
[//]: # (```)
<img src="mermaid-diagram-2023-01-10-224319.svg">


| Item     | Type |       Description                           |
|-----------------|-------------------------|------------------------------------------|
| `instruction`     |   input  | Send by CLI or Web UI.|
| `starcry`         |   transformer  | Sets up all components and transforms instructions into commands. |
| `cmds`         |   queue  | Command that defines what needs to be generated (e.g., frame, video, raw image)|
| `generator`         |   transformer  | Evaluates the script related to the command, generates a job for rendering.|
| `jobs`         |   queue  | Job definition ready for rendering. |
| `renderer`         |   transformer  | Takes jobs from queue and renders them by delegating them to either local or remote workers. Places rendered frames in frames queue. |
| `frames`         |   queue  | Frames or partial frames(?) Double-check. |
| `streamer`         |   consumer  | Stiches together frames in the right sequence into video if needed, or outputs individual frames to disk as images. Or sends to Webserver?? Double-check. |
| `output`         |   output  | Either disk or Websockets. |

# CLI and UI

The command line interface can be used to render images, still frames, etc., basically everything.
The web-based UI is used for interactively working with scripts. At the time of writing you work with frames only in the UI, and the CLI has to be used to execute the final rendering of the video.

# Instructions and Commands

The pipeline has a few different types of entry points, instructions send to the engine.

| Instruction     |      Command            |    Description                           |
|-----------------|-------------------------|------------------------------------------|
| `get_image`     | `command_get_image`     | Fetches PNG image of specific frame. Used by remote rendering workers when communicating to main render and by UI `ImageHandler` for fetching server-rendered frames as a PNG image (for use-cases where further processing is not needed). |
| `get_shapes`    | `command_get_shapes`    | Used by UI `ShapesHandler`, in case local WASM rendering + display is used. |
| `get_objects`   | `command_get_objects`   | Used by UI `ObjectsHandler`, in case meta-information about objects is needed. |
| `get_bitmap`    | `command_get_bitmap`    | Used by UI `BitmapHandler`, in case local WASM display-only is used. |
| `get_video`     | `command_get_video`     | Used by CLI to render videos. |
| `get_raw_image` | `command_get_raw_image` | Used by CLI to render raw images in OpenEXR format. |
| `get_raw_video` | N/A                     | Not implemented, but this should output raw images for each frame. |

Clarifications:
* For each instruction type there is a dedicated command object.
* `main.cpp` or the web ui will call  `starcry::add_image_command`, which in turn instantiates the correct command and saves it in the `cmds` storage.
TODO: The plan is to refactor this part, as it's a little bit unclear how each instruction maps to which overload.
* TODO: `get_objects` meta-data should include something like max. distance travelled in frame.

# WebSocket Handlers

The Web UI communicates with various websockets, each dedicated to parsing certain type of messages. In hindsight a fewer simultaneous connections per client makes more sense to me, but at the time of writing I had not too much experience with websockets.

| Handler         | Input                   | Executes |    Output                                |
|-----------------|-------------------------|------------------|------------------------|
| `BitmapHandler` | `{filename: "foo.js", frame: 1 }` | `get_bitmap` + ViewPoint | `RGBA8888` binary data. (Viewpoint can also define `raw` and `preview`, `raw` will be written to disk as a side-effect.) |
| `ImageHandler` | `"foo.js 1"` | `get_bitmap` (raw=false, preview=false) | `PNG` image binary data in `std::string buffer`). |
| `ObjectsHandler` | `"foo.js 1"` | `get_objects` (raw=false, preview=false) | `JSON` data containing all objects in `std::string buffer`). |
| `ScriptHandler` | - | on connect | Sends active script filename to client. |
|  ...  | `"open foo.js"` | reads file (e.g., `input/foo.js`) contents | Javascript file contents as string |
| ... | `"list"` | reads input directory | `JSON` data containing all files with file name, size and modified. |
| `ShapesHandler` | `"foo.js 1"` | `get_shapes` (raw=false, preview=false) | `JSON` serialized `data::job` (which is input for the Renderer) in `std::string buffer`). |
| `StatsHandler` | - | - | Asynchronously server-side can send statistics to client (unidirectional). See `webserver::send_stats`. |
| `ViewPointHandler` | `{operation: "set", ...}` | Updates `data::viewpoint` struct in `starcry` class. | - |
| ... | `{operation: "get"}` | Retrieves `data::viewpoint` struct from `starcry` class. | `JSON` data containing `data::viewpoint` struct from `starcry` class. |

* Input and Output is fully asynchronous.
* TODO: `JSON` can be used more consistently, where in some places text messages are being used.

## Example: ObjectHandler - serialized objects

Put example here.

## Example: ViewPoint - serialized JSON

Put example here.

## Example: StatsHandler - serialized stats

Put example here.

# The generator

The generator is the most complicated part of the Starcry engine. It is single-threaded and tightly integrated with the V8 javascript engine. To avoid it being too much of a bottleneck the input for the generator is as declarative as possible, so that most of the heavy-lifting can be done in C++. Some more work can be done in the future to make the V8 integration lighter, but for now the integration is pretty heavy.

`generator::generate_frame`

A scene determines which objects are involved at any given frame.

Besides `objects` there are three buckets that contain instances of `objects`.
Initially all three will contain the same instances (copies!) of the same object.

When processing a frame, these are their purposes:

* `instances` - current frame instances
* `instances_next` - next frame instances
* `instances_intermediate` - intermediate values

The third bucket makes sense if you consider that a single frame may be split up in ten sub-frames. For a 25 FPS video, a single frame accounts for 40 milliseconds of video. With ten sub-frames, we advance 4 milliseconds ten times, to get smooth motion. In this case the intermediate instances keep track of current state changes on the instances. Once the ten sub frames are rendered correctly, they will be committed to instances_next.

Please note that while generating frames, there is logic that determines if objects moved too much, and we can abort, set finer granularity (more sub-frames) and start over. For this reason it's necessary to keep separate buckets, so we can always revert any state we've made.

## Script layout

| Component       |    Description                           |
|-----------------|------------------------------------------|
| `gradients`     | List of gradients that can be referenced by objects. |
| `textures`      | List of textures that can be referenced by objects. |
| `objects`       | List of objects, each object can have sub-objects.  |
| `video`         | Properties of the video, such as resolution, and other settings. |
| `preview`       | Optional overrides for properties defined in video, when rendering previews. |
| `scenes`        | Scenes define which objects get instantiated and when. |

### gradients

TODO

### textures

TODO

### objects

TODO

### video

TODO

### preview

TODO

### scenes

TODO

## Memory usage

### Memory usage of objects

Starcry projects typically spawn a bunch of objects like circles, lines, etc. These objects used to exist both in C++ and inside V8 as Javascript objects.
Recent refactorings have made it so that for each *type* of object has only one V8 javascript instance counterpart, called a "bridge object" (one bridge for circles, one for lines, and so on).
When we need to process Javascript logic for any specific object (e.g., init() or time()), we take the appropriate bridge object, wire it with the C++ object and invoke the function call on it.
Side-effects produced by this execution will be automatically made to the C++ object, because all the javascript properties have been linked to the C++ object via the bridge. 

V8 is garbage collected, and its memory usage is also printed after processing each frame.
It should stay relatively consistent around 1 or 2 GB at the time of writing. We have to be more concerned with the memory usage of the C++ objects.

Objects are std::variants in the code-base, so each object has the same memory usage of at least 688 bytes at the time of writing.
Let's round it up to 700 for now, and assume 100.000 objects in some project (which is quite a lot). This should take around 66.75 MB of memory (100.000 * 700 bytes).
Let's assume some objects have a lot of additional properties set, and round it up again, very generously, up to 100 MB for 100.000 objects.

As explained in the generator section of this document, we have three copies of each object, current, intermediate and next. So we have to multiply by three to get 300 MB of memory usage for 100.000 objects.

Now let's say the frame in question is moving the objects 30 pixels to the right, resulting in 30-sub frames, this will result in 30 * 300 MB = 9 GB of memory because of all the necessary copies.
That's a lot, but not the worst of it. If we do chunked rendering, let's say 16 chunks because we want to render quickly with 16 threads (`starcry -t 16 -c 16`), we get: 9 * 16 = 144 GB of memory usage.
When we "chunk" a job into separate jobs, We create a separate job object for each chunk, each containing a copy of all the shapes that need to be rendered. If this is the bottleneck, the advise is to render *without* chunks (`starcry -t 16 -c 1`).

But in the case of a video, on a system with 16 cores (`starcry -t 16`) each core will process a different frame with potentially each 9 GB of shapes data is also a potential issue. For this there are a few solutions:
* Use fewer threads (`starcry -t 1`, to use just one. Having the others idle unfortunately.). You can still perhaps improve rendering speed by using remote workers.
* Use smaller queues (`starcry --concurrent-jobs 1`). The default of 10 would allow the generator part of the system to queue up max. 10 jobs for threads to pick up for rendering.

### Memory usage of bitmaps

In the case of a 1920x1080 image, we save doubles (4 bytes) per R,G,B,A channel by default. This is pretty hefty, already ~ 8 MB per frame. The way we render motion blur objects though, the absolute worst-case for an object in motion, could be 30 sub-frames where each pixel on the screen is colored. This results in 30 * 8 MB = 240 MB of memory (stored in `std::unordered_map`s). Understandably a 4K image and also more sub-frames can result in heavy memory usage.

We can deal with this by also 'chunking' here, but without parallelization, unfortunately.
We can render 1/16th of the image, we will iterate each object, do all motion blurring and necessary buffering of pixels for that region of the image, and once done we save only the result of that region.
Then we continue with the 2nd region, and so on. We will now iterate 16 times over all objects, and clear the unordered_maps after each iteration, this introduces slowness, but we will be less taxing on memory.

This optimization is done automatically in case sub-frames are used (motion blur is needed) and only one chunk is specified by the user.
The value to use is the number of sub-frames in the frame, so if the image is blurring over 30 sub-frames, we will divide the bitmap in 30, and then sequentially render each region, and concatenate the results in the final bitmap.

## TODO

* For local threads, we can utilize smart pointers to have chunks use the same memory for jobs.
* Make sure that we are smart about chunking, doesn't make sense if we have already 9GB worth of objects: if the user specified `-c 16` overrule it with `-c 1` with a warning.