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

// #include <Magick++.h>
#include <curses.h>
#include <fmt/core.h>

#include "generator.h"
#include "starcry.h"
#include "util/scope_exit.hpp"

namespace po = ::boost::program_options;
// using namespace Magick;

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

WINDOW *mainwin;
volatile bool flag = true;

void cleanup_curses() {
  flag = false;
  delwin(mainwin);
  endwin();
  refresh();
  std::cout << "\nWelcome back..." << std::endl;
}

void my_handler(int s) {
  printf("Caught signal %d\n", s);
  cleanup_curses();
  exit(1);
}

class main_program {
private:
  po::variables_map vm;
  std::string output_file = "";
  size_t frame_of_interest = std::numeric_limits<size_t>::max();
  double rand_seed = std::numeric_limits<double>::max();
  po::options_description desc = std::string("Allowed options");
  std::string script = "input/test.js";
  size_t num_worker_threads = 1;
  size_t num_chunks = 1;
  std::string host = "localhost";

public:
  main_program(int argc, char *argv[]) {
    /*
    std::cout << "Switching to separate screen..." << std::endl;
    if ((mainwin = initscr()) == NULL) {
      fprintf(stderr, "Error initialising ncurses.\n");
      exit(EXIT_FAILURE);
    }
    mvaddstr(0, 0, "Welcome to Starcry!");
    refresh();

    timeout(1000);
    std::thread test([&]() {
      noecho();
      while (flag) {
        int c = getch();
        if (c != -1) {
          mvaddstr(1, 0, fmt::format("Last key pressed: {}  ", c).c_str());
        }
      }
    });

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    scope_exit se([&]() {
      cleanup_curses();
      std::cout << "Welcome back..." << std::endl;
      test.join();
    });
    */

    po::positional_options_description p;
    p.add("script", 1);
    p.add("output", 1);
    // clang-format off
    desc.add_options()
      ("help", "produce help message")
      ("script,s", po::value<std::string>(&script), "javascript file to use for processing")
      ("output,o", po::value<std::string>(&output_file), "filename for video output (default output_{seed}_{width}x{height}.h264)")
      ("frame,f", po::value<size_t>(&frame_of_interest), "specific frame to render and save as 8-bit PNG and 32-bit EXR file")
      ("seed", po::value<double>(&rand_seed), "override the random seed used")
      ("num_threads,t", po::value<size_t>(&num_worker_threads), "number of local render threads (default 1)")
      ("num_chunks,c", po::value<size_t>(&num_chunks), "number of chunks to chop frame into (default 1)")
      ("server", "start server to allow dynamic renderers (default no)")
      ("client", po::value<std::string>(&host), "start client renderer, connect to host (default localhost)")
      ("gui", "render to graphical window")
      ("gui-only", "render to graphical window only (no video)")
      ("preview", "enable preview settings for fast preview rendering")
      ("javascript-only", "render only the jobs, nothing graphical")
      ("spawn-gui", "spawn GUI window (used by --gui, you probably don't need to call this)")
      ("stream", "start embedded webserver and stream HLS to webroot")
      ("interactive,i", "start embedded webserver and launch in interactive mode")
      ("pipeline,p", "non-interactive pipeline mode")
      ("perf", "run performance tests")
      ("compression", "enable pixel compression on rendered pixels")
      ("verbose,v", "enable verbose output (default no)")
      ("quiet,q", "disable verbose progress (default no)")
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

    bool is_interactive = vm.count("interactive");
    bool start_webserver = is_interactive == true;
    bool preview = vm.count("preview");

    // configure streaming
    if (vm.count("stream")) {
      if (output_file == "") {  // default
        output_file = "webroot/stream/stream.m3u8";
        // clean up left-over streaming artifacts
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
      std::cerr << "View stream here: http://localhost:18080/stream.html" << std::endl;
      start_webserver = true;
    }

    auto p_mode = ([&]() {
      if (vm.count("gui"))
        return starcry::render_video_mode::video_with_gui;
      else if (vm.count("gui-only"))
        return starcry::render_video_mode::gui_only;
      else if (vm.count("javascript-only"))
        return starcry::render_video_mode::javascript_only;
      else
        return starcry::render_video_mode::video_only;
    })();

    auto render_command = [&](starcry &sc) {
      if (is_interactive) {
        // in interactive mode commands come from the web interface
        return;
      }
      bool is_raw = vm.count("raw");
      if (frame_of_interest != std::numeric_limits<size_t>::max()) {
        // render still image
        sc.add_command(
            nullptr, script, instruction_type::get_raw_image, frame_of_interest, num_chunks, is_raw, preview);
      } else {
        // render video
        sc.add_command(nullptr, script, output_file, num_chunks, is_raw, preview);
      }
    };

    starcry::log_level level = starcry::log_level::info;
    if (vm.count("quiet")) {
      level = starcry::log_level::silent;
    } else if (vm.count("verbose")) {
      level = starcry::log_level::debug;
    }

    starcry sc(num_worker_threads,
               vm.count("server"),
               level,
               is_interactive,
               start_webserver,
               vm.count("compression"),
               p_mode,
               render_command,
               rand_seed != std::numeric_limits<double>::max() ? std::make_optional<double>(rand_seed) : std::nullopt);

    if (vm.count("client")) {
      sc.run_client(host);
    } else {
      sc.run_server();
    }
  }
};

int main(int argc, char *argv[]) {
  // InitializeMagick(*argv);
  main_program prog{argc, argv};
  return 0;
}
