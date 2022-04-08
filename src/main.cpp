/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <experimental/filesystem>
#include <iostream>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc11-extensions"
#endif  // __clang__

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "generator.h"
#include "starcry.h"
#include "util/logger.h"

#include "v8/v8-version.h"

namespace po = ::boost::program_options;

class main_program {
private:
  po::variables_map vm;
  po::options_description desc = std::string("Allowed options");
  /**
   * Please note that V8 can only be full initialized per process.
   * You can confirm this by modifying the hello-world example.
   * For this reason we have to pass this context into our objects
   * everywhere, and make sure we only have a single instance.
   */
  std::shared_ptr<v8_wrapper> context;
  starcry_options options;
  double rand_seed = std::numeric_limits<double>::max();

public:
  main_program(int argc, char *argv[]) {
    po::positional_options_description p;
    p.add("script", 1);
    p.add("output", 1);
    // clang-format off
    desc.add_options()
      ("help", "produce help message")
      ("script,s", po::value<std::string>(&options.script_file), "javascript file to use for processing")
      ("output,o", po::value<std::string>(&options.output_file), "filename for video output (default output_{seed}_{width}x{height}.h264)")
      ("frame,f", po::value<size_t>(&options.frame_of_interest), "specific frame to render and save as 8-bit PNG and 32-bit EXR file")
      ("frame-offset", po::value<size_t>(&options.frame_offset), "frame offset (used to skip rendering frames)")
      ("seed", po::value<double>(&rand_seed), "override the random seed used")
      ("num_threads,t", po::value<size_t>(&options.num_worker_threads), "number of local render threads (default 1)")
      ("num_chunks,c", po::value<size_t>(&options.num_chunks), "number of chunks to chop frame into (default 1)")
      ("server", "start server to allow dynamic renderers (default no)")
      ("client", po::value<std::string>(&options.host), "start client renderer, connect to host (default localhost)")
      ("no-gui", "disable render to graphical window")
      ("no-output", "disable producing any output (video or stream)")
      ("no-render", "disable all rendering (e.g. for performance testing js)")
      ("preview", "enable preview settings for fast preview rendering")
      ("stream", "start embedded webserver and stream HLS to webroot")
      ("no-webserver", "do not start embedded webserver")
      ("interactive,i", "start in interactive mode (user will input through webserver)")
      ("pipeline,p", "non-interactive pipeline mode")
      ("perf", "run performance tests")
      ("compression", "enable pixel compression on rendered pixels")
      ("verbose,v", "enable verbose output (default no)")
      ("quiet,q", "disable verbose progress (default no)")
      ("notty,n", "disable terminal (default tty is assumed)")
      ("caching", "enable caching (experimental feature)")
      ("raw,r", "write raw 32-bit EXR frames (default no)");
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

    options.webserver = !vm.count("no-webserver");
    options.interactive = vm.count("interactive");
    options.preview = vm.count("preview");
    options.enable_remote_workers = vm.count("server");
    options.gui = !vm.count("no-gui");
    options.output = !vm.count("no-output");
    options.render = !vm.count("no-render");
    options.notty = vm.count("notty");
    options.compression = vm.count("compression");
    options.rand_seed =
        (rand_seed != std::numeric_limits<double>::max()) ? std::optional<double>(rand_seed) : std::nullopt;

    if (vm.count("quiet")) {
      options.level = log_level::silent;
    } else if (vm.count("verbose")) {
      options.level = log_level::debug;
    }

    if (vm.count("stream")) configure_streaming();

    context = std::make_shared<v8_wrapper>(options.script_file);
    starcry sc(options, context);
    if (vm.count("caching")) {
      sc.features().caching = true;
    }

    if (vm.count("client")) {
      sc.run_client(options.host);
    } else {
      sc.setup_server();

      if (!options.interactive) {
        bool is_raw = vm.count("raw");
        if (options.frame_of_interest != std::numeric_limits<size_t>::max()) {
          sc.add_image_command(nullptr,
                               options.script_file,
                               instruction_type::get_raw_image,
                               options.frame_of_interest,
                               options.num_chunks,
                               is_raw,
                               options.preview,
                               true,
                               options.output_file);
        } else {
          sc.add_video_command(nullptr,
                               options.script_file,
                               options.output_file,
                               options.num_chunks,
                               is_raw,
                               options.preview,
                               options.frame_offset);
        }
      }

      sc.run_server();
    }
  }

  void configure_streaming() {
    if (options.output_file.empty()) {  // default
      options.output_file = "webroot/stream/stream.m3u8";
      cleanup_left_over_streaming_files();
    }
    std::cerr << "View stream here: http://localhost:18080/stream.html" << std::endl;
    options.webserver = true;
  }

  static void cleanup_left_over_streaming_files() {
    namespace fs = std::experimental::filesystem;
    const fs::path stream_path{"webroot/stream"};
    for (const auto &entry : fs::directory_iterator(stream_path)) {
      const auto filename = entry.path().filename().string();
      // Note to self in the future: non-experimental filesystem can do:
      //  if (entry.is_regular_file())...
      if (filename.rfind("stream.m3u8", 0) == 0) {
        std::cerr << "cleaning up old file: " << filename << std::endl;
        fs::remove(entry.path());
      }
    }
  }
};

int main(int argc, char *argv[]) {
  logger(DEBUG) << "Welcome to Starcry" << std::endl;
  logger(DEBUG) << "Integrated with v8 " << V8_MAJOR_VERSION << "." << V8_MINOR_VERSION << " build: " << V8_BUILD_NUMBER
                << " patch lvl: " << V8_PATCH_LEVEL << " candidate: " << V8_IS_CANDIDATE_VERSION << std::endl;

  main_program prog{argc, argv};
  return 0;
}
