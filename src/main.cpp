/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bitset>
#include <iostream>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc11-extensions"
#endif  // __clang__

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "starcry.h"
#include "webserver.h"

namespace po = ::boost::program_options;

class main_program {
private:
  po::variables_map vm;
  std::string output_file = "output.h264";
  size_t frame_of_interest = std::numeric_limits<size_t>::max();
  po::options_description desc = std::string("Allowed options");
  std::string script = "input/test.js";

public:
  main_program(int argc, char *argv[]) {
    po::positional_options_description p;
    p.add("script", 1);
    p.add("output", 1);
    // clang-format off
    desc.add_options()
      ("help", "produce help message")
      ("script,s", po::value<std::string>(&script), "javascript file to use for processing")
      ("output,o", po::value<std::string>(&output_file), "filename for video output (default output.h264)")
      ("frame,f", po::value<size_t>(&frame_of_interest), "specific frame to render and save as BMP file")
      ("gui", "render to graphical window")
      ("gui-only", "render to graphical window only (no video)")
      ("spawn-gui", "spawn GUI window (used by --gui, you probably don't need to call this)")
      ("stream", "start embedded webserver and stream HLS to webroot")
      ("perf", "run performance tests");
    // clang-format on

    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    try {
      po::notify(vm);
    } catch (po::error &ex) {
      std::cerr << "Error: " << ex.what() << std::endl;
      std::cerr << desc << std::endl;
      std::exit(1);
    }
    if (vm.count("help")) {
      std::cerr << argv[0] << " [ <script> ] [ <output> ]" << std::endl << std::endl << desc << "\n";
      std::exit(1);
    }

    starcry sc(script, output_file);

    // render still image
    if (frame_of_interest != std::numeric_limits<size_t>::max()) {
      auto res = sc.render_frame(frame_of_interest);
      std::cout << "Render time: " << res.time << " seconds, definition size: " << res.definition_bytes << " bytes"
                << std::endl;
      return;
    }

    // run some benchmark tests
    if (vm.count("perf")) {
      sc.run_benchmarks();
      return;
    }

    // configure streaming
    std::unique_ptr<webserver> ws;
    if (vm.count("stream")) {
      ws = std::make_unique<webserver>();
      sc.configure_streaming();
      std::cerr << "Stream URL: http://localhost:18080/" << std::endl;
    }

    // render video
    auto fps = ([&](){
      if (vm.count("gui"))
        return sc.render_video(starcry::render_video_mode::video_with_gui);
      else if (vm.count("gui-only"))
        return sc.render_video(starcry::render_video_mode::gui_only);
      else
        return sc.render_video(starcry::render_video_mode::video_only);
    })();
    std::cout << "Rendering done, average FPS: " << fps << std::endl;
  }
};

int main(int argc, char *argv[]) {
  main_program prog{argc, argv};
  return 0;
}
