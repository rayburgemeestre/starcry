#include <bitset>
#include <experimental/filesystem>
#include <iostream>

/**
 * This warning is driving me crazy,
 *
 * In file included from /home/trigen/projects/starcry/src/main.cpp:6:
 * In file included from /opt/cppse/build/boost/include/boost/program_options/options_description.hpp:16:
 * In file included from /opt/cppse/build/boost/include/boost/shared_ptr.hpp:17:
 * In file included from /opt/cppse/build/boost/include/boost/smart_ptr/shared_ptr.hpp:28:
 * In file included from /opt/cppse/build/boost/include/boost/smart_ptr/detail/shared_count.hpp:29:
 * In file included from /opt/cppse/build/boost/include/boost/smart_ptr/detail/sp_counted_base.hpp:45:
 *
 * /opt/cppse/build/boost/include/boost/smart_ptr/detail/sp_counted_base_clang.hpp:29:9: warning: '_Atomic' is a C11
 * extension [-Wc11-extensions]
 */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wc11-extensions"
#endif  // __clang__

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <regex>
#include "actors/job_generator.h"
#include "actors/render_window.h"
#include "actors/renderer.h"
#include "actors/stdin_reader.h"
#include "actors/streamer.h"
#include "atom_types.h"
#include "caf/io/middleman.hpp"
#include "caf/logger.hpp"
#include "common.h"
#include "data/pixels.hpp"
#include "util/actor_info.hpp"
#include "util/settings.hpp"
#include "webserver.h"

namespace po = ::boost::program_options;

// crtmpserver
extern int main__(int argc, const char *argv[]);

using std::bitset;
using std::cerr;
using std::cout;
using std::ifstream;
using std::make_shared;
using std::pair;
using std::regex;
using std::shared_ptr;
using std::smatch;
using std::string;
using std::stringstream;
using std::vector;

volatile bool starcry_running = true;

class crtmpserver_wrapper {
public:
  crtmpserver_wrapper()
      : crtmpserver_thread_([&]() {
          int argc = 2;
          const char *argv[] = {"starcry", "crtmpserver.lua"};
          main__(argc, argv);
        }) {}
  ~crtmpserver_wrapper() { crtmpserver_thread_.join(); }

private:
  std::thread crtmpserver_thread_;
};

class main_program {
private:
  po::variables_map vm;
  // settings
  int worker_port = 0;
  size_t num_chunks = 1;   // number of chunks to split image size_to
  size_t num_workers = 1;  // number of workers for rendering
  string worker_ports;
  string dimensions;
  string output_file = "";
  uint32_t settings_ = 0;
  po::options_description desc = string("Allowed options");
  string script = "input/test.js";
  bool compress = false;
  bool rendering_enabled = true;
  string renderer_host = "localhost";
  int renderer_port = 10000;
  string remote_renderer_info;
  string remote_streamer_info;
  size_t max_jobs_queued_for_renderer = 1;
  int64_t save_image = -1;
  int64_t num_queue_per_worker = 1;

public:
  main_program(int argc, char *argv[]) {
    ::settings conf;
    conf.load();

    po::positional_options_description p;
    p.add("script", 1);
    p.add("output", 1);

    desc.add_options()("help", "produce help message")(
        "output,o", po::value<string>(&output_file), "filename for video output (default output.h264)")(
        "remote,r", po::value<string>(&worker_ports), "use remote workers for rendering")(
        "worker,w", po::value<int>(&worker_port), "start worker with specific ID")(
        "queue-max-frames,q", po::value<size_t>(&max_jobs_queued_for_renderer), "number of frames to queue")(
        "num-chunks,c", po::value<size_t>(&num_chunks), "number of jobs to split frame in (default 1)")(
        "num-workers,n", po::value<size_t>(&num_workers), "number of workers to use in render pool (default 8)")(
        "num-queue-worker,Q",
        po::value<int64_t>(&num_queue_per_worker),
        "number of items workers prefer in their queue (default 1)")("gui", "render to allegro5 window")(
        "preview", "open GUI preview window (writes state to $HOME/.starcry.conf)")(
        "spawn-gui", "spawn GUI window (used by --gui, you probably don't need to call this)")(
        "no-video-output", "disable video output using ffmpeg")(
        "no-rendering", "disable rendering (useful for testing javascript performance)")(
        "save-image", po::value<int64_t>(&save_image), "save image to disk")(
        "expose-renderer", po::value<int>(&renderer_port), "expose renderer on given port")(
        "webserver", "start embedded webserver")("crtmpserver", "start embedded crtmpserver for rtmp streaming")(
        "stream,stream-hls", "start embedded webserver, and stream hls to webroot")(
        "stream-rtmp", "start embedded webserver, crtmpserver and stream flv to it on local host (deprecated)")(
        "stdin", "read from stdin and send this to job generator actor")(
        "dimensions,dim", po::value<string>(&dimensions), "specify canvas dimensions i.e. 1920x1080")(
        "script,s", po::value<string>(&script), "javascript file to use for processing")(
        "compress", po::value<bool>(&compress), "compress and decompress frames to reduce I/O")(
        "trace", "enable tracing (this requires CAF to be compiled with the appropriate flags)");

    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    try {
      po::notify(vm);
    } catch (po::error &ex) {
      std::cerr << "Error: " << ex.what() << std::endl;
      std::cerr << desc << std::endl;
      std::exit(1);
    }

    caf::exec_main_init_meta_objects<caf::id_block::starcry, io::middleman>();
    caf::core::init_global_meta_objects();
    actor_system_config cfg;
    cfg.load<io::middleman>();

    if (vm.count("trace")) {
      cfg.set("logger.file-name", "actor_log_[PID]_[TIMESTAMP]_[NODE].log");
      cfg.set("logger.file-format", "%r %c %p %a %t %C %M %F:%L %m%n");
      cfg.set("logger.file-verbosity", "trace");
      cfg.set("logger.console", "colored");
      cfg.set("logger.console-format", "%m");
      cfg.set("logger.console-verbosity", "debug");
    } else {
      cfg.set("logger.file-verbosity", "quiet");
      cfg.set("logger.console-verbosity", "quiet");
    }

    actor_system system(cfg);

    rendering_enabled = !vm.count("no-rendering");
    if (vm.count("script")) {
      cerr << "Script: " << vm["script"].as<string>() << "\n";
    }
    if (vm.count("help")) {
      cerr << desc << "\n";
      std::exit(1);
    }
    compress = vm.count("compress");

    // determine outputs
    bitset<32> streamer_settings(settings_);
    if (!vm.count("no-video-output")) {
      cerr << "Enabling video output using ffmpeg.." << endl;
      streamer_settings.set(streamer_ffmpeg, true);
    }
    if (vm.count("gui")) {
      cerr << "Enabling video output to real-time window" << endl;
      streamer_settings.set(streamer_sfml, true);
    }
    if (vm.count("preview")) {
      auto client = system.middleman().remote_actor("127.0.0.1", conf.user.gui_port);
      if (!client) {
        if (0 != ::system((std::string(argv[0]) + " --spawn-gui &").c_str())) {
          cerr << "System call to --spawn-gui failed.." << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        conf.load();
      }
      cerr << "Enabling video output to preview window on port " << conf.user.gui_port << ".." << endl;
      streamer_settings.set(streamer_allegro5, true);
    }
    scoped_actor s{system};
    if (vm.count("spawn-gui")) {
      cerr << "launching render output window" << endl;
      auto w = system.spawn(render_window, conf.user.gui_port);
      s->await_all_other_actors_done();
      std::exit(0);
    }

    if (worker_port) cerr << "worker_port=" << worker_port << endl;
    if (num_chunks) cerr << "num_chunks=" << num_chunks << endl;
    if (num_workers) cerr << "num_workers=" << num_workers << endl;

    if (worker_port) {
      // TODO: hardcoded
      auto w = system.spawn(remote_worker, worker_port, "localhost", 10000, num_queue_per_worker);
      s->send(w, start_v);
      s->await_all_other_actors_done();
      std::exit(0);
    }
    uint32_t canvas_w = 480;
    uint32_t canvas_h = 320;
    if (!dimensions.empty()) {
      regex range("([0-9]+)x([0-9]+)");
      smatch m;
      if (std::regex_match(dimensions, m, range) && m.size() == 3) {
        {
          stringstream os(m[1]);
          os >> canvas_w;
        }
        {
          stringstream os(m[2]);
          os >> canvas_h;
        }
      }
    }
    bool use_stdin = vm.count("stdin");
    size_t use_fps = 25;
    std::string stream_mode = "";
    shared_ptr<webserver> ws;
    if (vm.count("stream") || vm.count("stream-rtmp") || vm.count("stream-hls") || vm.count("webserver")) {
      ws = make_shared<webserver>();
    }
    shared_ptr<crtmpserver_wrapper> cw;
    if (vm.count("stream-rtmp") || vm.count("crtmpserver")) {
      cw = make_shared<crtmpserver_wrapper>();
      ;
    }
    if (vm.count("stream") || vm.count("stream-hls")) {
      stream_mode = "hls";
      if (output_file.empty()) {
        output_file.assign("webroot/stream/stream.m3u8");

        // clean up left-over streaming artifacts
        namespace fs = std::experimental::filesystem;
        const fs::path stream_path{"webroot/stream"};
        for (const auto &entry : fs::directory_iterator(stream_path)) {
          const auto filename = entry.path().filename().string();
          // non-experimental can do:
          // if (entry.is_regular_file())...
          if (filename.rfind("stream.m3u8", 0) == 0) {
            std::cerr << "cleaning up old file: " << filename << std::endl;
            fs::remove(entry.path());
          }
        }
      }
      std::cerr << "Stream URL: http://localhost:18080/" << std::endl;
    }
    if (vm.count("stream-rtmp")) {
      stream_mode = "rtmp";
      if (output_file.substr(0, 4) != "rtmp") {
        output_file.assign("rtmp://localhost/flvplayback/video");
      }
    }
    if (output_file.empty()) output_file = "output.h264";

    // auto jobstorage = system.spawn(job_storage);
    auto generator =
        system.spawn<detached>(job_generator, script, canvas_w, canvas_h, use_stdin, rendering_enabled, compress);
    // generator links to job storage
    actor streamer_ = spawn_actor_local_or_remote(
        system, streamer, string("streamer"), string("use-remote-streamer"), remote_streamer_info, -1);

    actor renderer_ = spawn_actor_local_or_remote(
        system, renderer, string("renderer"), string("use-remote-renderer"), remote_renderer_info, renderer_port);
    auto output_settings = uint32_t(streamer_settings.to_ulong());
    auto stdin_reader_ = system.spawn(stdin_reader, generator);

    s->request(generator, infinite, initialize_v)
        .receive(
            [&](size_t bitrate, bool use_stdin_, size_t use_fps_, bool realtime_) {
              use_stdin = use_stdin_;
              use_fps = use_fps_;
              s->send(streamer_,
                      initialize_v,
                      int(conf.user.gui_port),
                      string(output_file),
                      bitrate,
                      use_fps,
                      output_settings,
                      stream_mode);
              s->send(renderer_, initialize_v, streamer_, generator, save_image, realtime_);
            },
            [=](error &err) { std::exit(2); });
    s->send(renderer_, start_v, num_workers, num_queue_per_worker);
    s->send(generator, start_v, std::max(max_jobs_queued_for_renderer, num_workers), num_chunks, renderer_);

    if (use_stdin) {
      s->send(stdin_reader_, start_v);
    }

    actor_info streamer_info{"streamer", streamer_};
    actor_info generator_info{"generator", generator};
    actor_info renderer_info{"renderer", renderer_};
    actor_info stdin_reader_info{"stdin_reader", stdin_reader_};

    while (streamer_info.running()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      if (rendering_enabled) {
        s->send<message_priority::high>(renderer_, show_stats_v);
        s->send<message_priority::high>(generator, show_stats_v);
      } else {
        s->send<message_priority::high>(generator, show_stats_v);
      }
      /*
      s->send<message_priority::high>(streamer_, debug_v);
      s->send<message_priority::high>(generator, debug_v);
      s->send<message_priority::high>(renderer_, debug_v);
      if (use_stdin) {
        s->send<message_priority::high>(stdin_reader_, debug_v);
      }
       */
    }
    if (renderer_info.running()) s->send<message_priority::high>(renderer_, terminate__v);
    if (generator_info.running()) s->send<message_priority::high>(generator, terminate__v);
    s->await_all_other_actors_done();
    starcry_running = false;
  }

  bool extract_host_port_string(caf::actor_system &system, string host_port_str, string *host_ptr, int *port_ptr) {
    string &host = *host_ptr;
    int &port = *port_ptr;

    auto pos = host_port_str.find(":");
    if (pos == string::npos) {
      return false;
    }
    host = host_port_str.substr(0, pos);
    port = atoi(host_port_str.substr(pos + 1).c_str());
    return true;
  }

  template <typename T>
  actor spawn_actor_local_or_remote(caf::actor_system &system,
                                    T *actor_behavior,
                                    string actor_name,
                                    string cli_flag_param,
                                    string cli_flag_value,
                                    int port) {
    std::optional<actor> actor_ptr;
    if (vm.count(cli_flag_param)) {
      string host;
      int port = 0;
      if (!extract_host_port_string(system, cli_flag_value, &host, &port)) {
        cerr << "parameter for --" << cli_flag_param << " is invalid, please specify <host>:<port>, "
             << "i.e. 127.0.0.1:11111." << endl;
      }
      cerr << "using remote " << actor_name << " at: " << host << ":" << port << endl;
      auto p = system.middleman().remote_actor(host, port);
      if (!p) {
        cerr << "connecting to " << actor_name << " failed: " << system.render(p.error()) << endl;
      }
      actor_ptr = *p;
    } else {
      std::optional<size_t> the_port;
      if (port != -1) {
        the_port = std::optional<size_t>(port);
      }
      cerr << "spawning local " << actor_name << endl;
      actor_ptr = system.spawn<caf::spawn_options::priority_aware_flag>(actor_behavior, the_port);
    }
    return *actor_ptr;
  }
};

int main(int argc, char *argv[]) {
  main_program prog{argc, argv};
  return 0;
}
