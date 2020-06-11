#if 1 == 2
#include "caf/all.hpp"

using namespace caf;

CAF_BEGIN_TYPE_ID_BLOCK(test, first_custom_type_id)
CAF_ADD_ATOM(test, test)
CAF_END_TYPE_ID_BLOCK(test)

behavior bar_actor(event_based_actor* self) {
  return {
    [=](test) {
      std::cout << "test1" << std::endl;
    },
    [=](uint32_t value) {
      std::cout << "test2: " << value << std::endl;
    }
  };
}

behavior foo_actor(event_based_actor* self) {
  auto bar = self->spawn(bar_actor);
  bar->add_link(self);
  self->link_to(bar);
  self->send(bar, test_v); // segfaults

  return {
    [=](uint32_t width) {
      self->send(bar, test_v); // hangs (when by itself)
      self->send(bar, width);  // segfaults
    },
  };
}

void caf_main(actor_system& system) {
  scoped_actor s{system};
  auto foo = system.spawn(foo_actor);
  s->send(foo, uint32_t(123));
  s->await_all_other_actors_done();
}

// creates a main function for us that calls our caf_main
CAF_MAIN(caf::id_block::test)
#elif 2 == 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v8/libplatform/libplatform.h"
#include "v8/v8.h"

int main(int argc, char* argv[]) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);

    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);

    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);

    {
      // Create a string containing the JavaScript source code.
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8(isolate, "'Hello' + ', World!'",
                                  v8::NewStringType::kNormal)
              .ToLocalChecked();

      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();

      // Run the script to get the result.
      v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

      // Convert the result to an UTF8 string and print it.
      v8::String::Utf8Value utf8(isolate, result);
      printf("%s\n", *utf8);
    }

    {
      // Use the JavaScript API to generate a WebAssembly module.
      //
      // |bytes| contains the binary format for the following module:
      //
      //     (func (export "add") (param i32 i32) (result i32)
      //       get_local 0
      //       get_local 1
      //       i32.add)
      //
      const char* csource = R"(
        let bytes = new Uint8Array([
          0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01,
          0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
          0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09, 0x01,
          0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b
        ]);
        let module = new WebAssembly.Module(bytes);
        let instance = new WebAssembly.Instance(module);
        instance.exports.add(3, 4);
      )";

      // Create a string containing the JavaScript source code.
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8(isolate, csource, v8::NewStringType::kNormal)
              .ToLocalChecked();

      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();

      // Run the script to get the result.
      v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

      // Convert the result to a uint32 and print it.
      uint32_t number = result->Uint32Value(context).ToChecked();
      printf("3 + 4 = %u\n", number);
    }
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}

#elif 1 == 2
//# minimal v8 ex

// workaround by Ray
#include "v8/libplatform/libplatform.h"
#include "v8/v8.h"
#include <v8pp/context.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << " I'm on line" << __LINE__ << std::endl;
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  std::cout << " I'm on line" << __LINE__ << std::endl;
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::cout << " I'm on line" << __LINE__ << std::endl;
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  std::cout << " I'm on line" << __LINE__ << std::endl;
  v8::V8::InitializePlatform(platform.get());
  std::cout << " I'm on line" << __LINE__ << std::endl;
  v8::V8::Initialize();
  std::cout << " I'm on line" << __LINE__ << std::endl;

  v8pp::context context;
  //context.set_lib_path("path/to/plugins/lib");
  std::cout << " I'm on line" << __LINE__ << std::endl;
  // script can now use require() function. An application
  // that uses v8pp::context must link against v8pp library.
  v8::HandleScope scope(context.isolate());
  std::cout << " I'm on line" << __LINE__ << std::endl;
  auto res = context.run_file("some_file.js");
  v8::String::Utf8Value str(context.isolate(), res);
  std::cout << std::string(*str) << std::endl;
  std::cout << " I'm on line" << __LINE__ << std::endl;
}


#elif 1 == 3
//# minimal caf example

#include <string>
#include <iostream>

#include "caf/all.hpp"

/*
namespace std{
	inline namespace __1 {
		bad_function_call::~bad_function_call() _NOEXCEPT {}
		const char * bad_function_call::what() const _NOEXCEPT {}
	}
}
*/

using std::endl;
using std::string;

using namespace caf;

behavior mirror(event_based_actor* self) {
  // return the (initial) actor behavior
  return {
    // a handler for messages containing a single string
    // that replies with a string
    [=](const string& what) -> string {
      // prints "Hello World!" via aout (thread-safe cout wrapper)
      aout(self) << what << endl;
      // reply "!dlroW olleH"
      return string(what.rbegin(), what.rend());
    }
  };
}

void hello_world(event_based_actor* self, const actor& buddy) {
  // send "Hello World!" to our buddy ...
  self->request(buddy, std::chrono::seconds(10), "Hello World!").then(
    // ... wait up to 10s for a response ...
    [=](const string& what) {
      // ... and print it
      aout(self) << what << endl;
    }
  );
}

void caf_main(actor_system& system) {
  // create a new actor that calls 'mirror()'
  auto mirror_actor = system.spawn(mirror);
  // create another actor that calls 'hello_world(mirror_actor)';
  system.spawn(hello_world, mirror_actor);
  // system will wait until both actors are destroyed before leaving main
}

// creates a main function for us that calls our caf_main
CAF_MAIN()

#else
/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#if WORKER_ONLY
#include "common.h"
#include "actors/renderer.h"
#include "caf/io/middleman.hpp"
#include "data/pixels.hpp"
#include "data/job.hpp"
int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Usage " << argv[0] << " <worker_port>" << endl << endl;
        return 1;
    }
    actor_system_config cfg;

    cfg.add_message_type<data::job>("data::job");
    cfg.add_message_type<data::pixel_data>("data::pixel_data");
    cfg.add_message_type<data::pixel_data2>("data::pixel_data2");
    cfg.add_message_type<std::vector<uint32_t>>("vector<uint32_t>");
    cfg.load<io::middleman>();

    actor_system system(cfg);
    // TODO: remote renderer and/or streamer is not yet supported here
    auto w = system.spawn(remote_worker, atoi(argv[1]), "", 0, "", 0);
    return 0;
}
#else
#include <iostream>

#include "common.h"
#include "atom_types.h"
#include "actors/job_generator.h"
#include "actors/renderer.h"
#include "actors/streamer.h"
#include "actors/render_window.h"
#include "actors/stdin_reader.h"
#include "data/pixels.hpp"
#include "data/job.hpp"
#include "util/actor_info.hpp"
#include "util/settings.hpp"
#include "webserver.h"
#include "caf/io/middleman.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <regex>
#include <bitset>
//
// workaround by Ray
#include "v8/libplatform/libplatform.h"
#include "v8/v8.h"
#include <v8pp/context.hpp>


//#include "caf/io/max_msg_size.hpp"
//caf::io::max_msg_size(std::numeric_limits<uint32_t>::max());

namespace po = ::boost::program_options;

extern int main__(int argc, const char *argv[]);

#include <bitset>
using std::bitset;
using std::vector;
using std::pair;
using std::ifstream;
using std::string;
using std::cout;
using std::cerr;
using std::regex;
using std::smatch;
using std::stringstream;
using std::shared_ptr;
using std::make_shared;

volatile bool starcry_running = true;

class crtmpserver_wrapper
{
public:
    crtmpserver_wrapper() :
        crtmpserver_thread_([&]() {
            int argc = 2;
            const char *argv[] = { "starcry", "crtmpserver.lua" };
            main__(argc, argv);
        })
    {
    }
    ~crtmpserver_wrapper() {
        crtmpserver_thread_.join();
    }
private:
    std::thread crtmpserver_thread_;
};

class main_program
{
private:
    actor_system &system;
    po::variables_map vm;
    // settings
    int worker_port               = 0;
    size_t num_chunks             = 1; // number of chunks to split image size_to
    size_t num_workers            = 8; // number of workers for rendering
    string worker_ports;
    string dimensions;
    string output_file            = "";
    uint32_t settings_            = 0;
    po::options_description desc  = string("Allowed options");
    string script                 = "test.js";
    bool compress                 = false;
    bool javascript_enabled       = true;
    bool rendering_enabled        = true;
    string renderer_host;
    string streamer_host;
    int renderer_port             = 0;
    int streamer_port             = 0;
    string remote_renderer_info;
    string remote_streamer_info;
    size_t max_jobs_queued_for_renderer = 1;

public:
    main_program(actor_system &system, int argc, char *argv[]) : system(system) {
//        // Initialize V8.
//        v8::V8::InitializeICUDefaultLocation(argv[0]);
//        v8::V8::InitializeExternalStartupData(argv[0]);
//        std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
//        v8::V8::InitializePlatform(platform.get());
//        v8::V8::Initialize();
//
//        v8pp::context context;
//        //context.set_lib_path("path/to/plugins/lib");
//        // script can now use require() function. An application
//        // that uses v8pp::context must link against v8pp library.
//        v8::HandleScope scope(context.isolate());
//        auto res = context.run_file("some_file.js");
//
//        v8::String::Utf8Value str(context.isolate(), res);
//        std::cout << std::string(*str) << std::endl;
//
////        std::exit(0);

//#include "v8.h"
//#include "libplatform/libplatform.h"

/* TEST PROGRAM THAT CAN TEST IF POINTER COMPRESSION CAUSES CRASH SEE */
/* https://bugs.chromium.org/p/v8/issues/detail?id=10041 */
/* detection fucking works great, now let's comment it out for now...
        std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform.get());
        v8::V8::Initialize();

        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        v8::Isolate *isolate = v8::Isolate::New(create_params);
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);

        v8::Local<v8::Context> ctx = v8::Context::New(isolate);
        v8::Context::Scope context_scope(ctx);

        v8::Local<v8::Array> arr = v8::Array::New(isolate, 3);
        arr->Set(ctx, 0, v8::Integer::New(isolate, 42));
        arr->Set(ctx, 1, v8::Integer::New(isolate, 84));
        arr->Set(ctx, 2, v8::Integer::New(isolate, 126));

        arr->IsString(); // <- segmentation fault
        std::exit(0);
        */



        ::settings conf;
        conf.load();

        po::positional_options_description p;
        p.add("script", 1);
        p.add("output", 1);

        //int renderer_port = 0;
        //bool use_seprender = false;
        desc.add_options()("help", "produce help message")
            ("output,o", po::value<string>(&output_file), "filename for video output (default output.h264)")
            ("remote,r", po::value<string>(&worker_ports), "use remote workers for rendering")
            ("worker,w", po::value<int>(&worker_port), "start worker on specified port")
            ("queue-max-frames", po::value<size_t>(&max_jobs_queued_for_renderer), "number of frames to queue")
            ("num-chunks,c", po::value<size_t>(&num_chunks), "number of jobs to split frame in (default 1)")
            ("num-workers,n", po::value<size_t>(&num_workers), "number of workers to use in render pool (default 8)")
            ("gui", "open GUI window (writes state to $HOME/.starcry.conf)")
            ("spawn-gui", "spawn GUI window (used by --gui, you probably don't need to call this)")
            ("no-video-output", "disable video output using ffmpeg")
            ("no-rendering", "disable rendering (useful for testing javascript performance)")
            ("spawn-renderer", po::value<int>(&renderer_port), "spawn renderer on given port")
            ("spawn-streamer", po::value<int>(&streamer_port), "spawn streamer on given port")
            ("use-remote-renderer", po::value<string>(&remote_renderer_info), "use remote renderer on given \"host:port\"")
            ("use-remote-streamer", po::value<string>(&remote_streamer_info), "use remote streamer on given \"host:port\"")
            ("webserver", "start embedded webserver")
            ("crtmpserver", "start embedded crtmpserver for rtmp streaming")
            ("stream,stream-hls", "start embedded webserver, and stream hls to webroot")
            ("stream-rtmp", "start embedded webserver, crtmpserver and stream flv to it on local host (deprecated)")
            ("stdin", "read from stdin and send this to job generator actor")
            ("dimensions,dim", po::value<string>(&dimensions), "specify canvas dimensions i.e. 1920x1080")
            ("script,s", po::value<string>(&script), "javascript file to use for processing")
            ("compress", po::value<bool>(&compress), "compress and decompress frames to reduce I/O")
            ;

        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        try {
            po::notify(vm);
        }
        catch (po::error &ex) {
            std::cout << "Error: " << ex.what() << std::endl;
            std::cout << desc << std::endl;
            std::exit(1);
        }
        rendering_enabled = ! vm.count("no-rendering");
        if (vm.count("script")) {
            cout << "Script: " << vm["script"].as<string>() << "\n";
        }
        if (vm.count("help")) {
            cout << desc << "\n";
            std::exit(1);
        }
        compress = vm.count("compress");

        // determine outputs
        bitset<32> streamer_settings(settings_);
        if (!vm.count("no-video-output")) {
            cout << "Enabling video output using ffmpeg.." << endl;
            streamer_settings.set(streamer_ffmpeg, true);
        }
        if (vm.count("gui")) {
            auto client = system.middleman().remote_actor("127.0.0.1", conf.user.gui_port);
            if (!client) {
                if (0 != ::system((std::string(argv[0]) + " --spawn-gui &").c_str())) {
                    cout << "System call to --spawn-gui failed.." << endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                conf.load();
            }
            cout << "Enabling video output to window on port " << conf.user.gui_port << ".." << endl;
            streamer_settings.set(streamer_allegro5, true);
        }
        scoped_actor s{system};
        if (vm.count("spawn-gui")) {
            cout << "launching render output window" << endl;
            auto w = system.spawn(render_window, conf.user.gui_port);
            s->await_all_other_actors_done();
            std::exit(0);
        }

        bool use_remote_workers    = vm.count("remote") || vm.count("worker");
        vector<pair<string, int>> workers_vec;
        if (use_remote_workers) {
            ifstream infile(worker_ports);
            string line;
            while (getline(infile, line)) {
                if (line[0] == ';' || line[0] == '#') continue;
                size_t pos = line.find(":");
                if (pos != string::npos)
                    line[pos] = ' ';
                pos = line.find(" ");
                if (pos == string::npos) {
                    cerr << "erroneous line in servers text: " << line << endl;
                    continue;
                }
                cout << "found server: " << line.substr(0, pos) << " and port: " << atoi(line.substr(pos + 1).c_str())
                     << endl;
                workers_vec.push_back(make_pair(line.substr(0, pos), atoi(line.substr(pos + 1).c_str())));
            }
        }

        if (use_remote_workers) cout << "use_remote_workers=true" << endl;
        if (worker_port) cout << "worker_port=" << worker_port << endl;
        if (num_chunks) cout << "num_chunks=" << num_chunks << endl;
        if (num_workers) cout << "num_workers=" << num_workers << endl;

        extract_host_port_string(remote_renderer_info, &renderer_host, &renderer_port);
        extract_host_port_string(remote_streamer_info, &streamer_host, &streamer_port);
        if (use_remote_workers && worker_port) {
            auto w = system.spawn(remote_worker, worker_port, renderer_host, renderer_port, streamer_host, streamer_port);
            s->await_all_other_actors_done();
            std::exit(0);
        }
        if (vm.count("spawn-renderer")) {
            auto w = system.spawn(renderer, renderer_port);
            s->await_all_other_actors_done();
            std::exit(0);
        }
        if (vm.count("spawn-streamer")) {
            auto w = system.spawn(streamer, streamer_port);
            s->await_all_other_actors_done();
            std::exit(0);
        }
        uint32_t canvas_w = 480;
        uint32_t canvas_h = 320;
        if (!dimensions.empty()) {
            regex range("([0-9]+)x([0-9]+)");
            smatch m;
            if (std::regex_match(dimensions, m, range) && m.size() == 3) {
                { stringstream os(m[1]); os >> canvas_w; }
                { stringstream os(m[2]); os >> canvas_h; }
            }
        }
        bool use_stdin  = vm.count("stdin");
        size_t use_fps = 25;
        std::string stream_mode = "";
        shared_ptr<webserver> ws;
        if (vm.count("stream") || vm.count("stream-rtmp") || vm.count("stream-hls") || vm.count("webserver")) {
            ws = make_shared<webserver>();
        }
        shared_ptr<crtmpserver_wrapper> cw;
        if (vm.count("stream-rtmp") || vm.count("crtmpserver")) {
            cw = make_shared<crtmpserver_wrapper>();;
        }
        if (vm.count("stream") || vm.count("stream-hls")) {
            stream_mode = "hls";
            if (output_file.empty()) {
                output_file.assign("webroot/stream/stream.m3u8");
            }
        }
        if (vm.count("stream-rtmp")) {
            stream_mode = "rtmp";
            if (output_file.substr(0, 4) != "rtmp") {
                output_file.assign("rtmp://localhost/flvplayback/video");
            }
        }
        if (output_file.empty())
            output_file = "output.h264";

        //auto jobstorage = system.spawn(job_storage);
        auto generator  = system.spawn<detached>(job_generator, script, canvas_w, canvas_h, use_stdin,
                                                                 rendering_enabled, compress);
        // generator links to job storage
        actor streamer_ = spawn_actor_local_or_remote(streamer, string("streamer"), string("use-remote-streamer"),
                                                      remote_streamer_info);

        actor renderer_ = spawn_actor_local_or_remote(renderer, string("renderer"), string("use-remote-renderer"),
                                                      remote_renderer_info);
        auto output_settings = uint32_t(streamer_settings.to_ulong());
        auto stdin_reader_ = system.spawn(stdin_reader, generator);

        s->request(generator, infinite, initialize_v).receive(
            [&](size_t bitrate, bool use_stdin_, size_t use_fps_) {
//                const auto &bitrate = std::get<0>(tpl);
//                const auto &use_stdin_ = std::get<1>(tpl);
                use_stdin = use_stdin_;
                use_fps   = use_fps_;
                s->send(streamer_, initialize_v, int(conf.user.gui_port), string(output_file), bitrate, use_fps, output_settings, stream_mode);
            },
            [=](error &err) {
                std::exit(2);
            }
        );
        s->send(renderer_, initialize_v, streamer_, generator, workers_vec, streamer_host, streamer_port);

        s->send(generator, start_v, max_jobs_queued_for_renderer, num_chunks, renderer_);
        s->send(renderer_, start_v, use_remote_workers ? workers_vec.size() : num_workers);

        if (use_stdin) {
            s->send(stdin_reader_, start_v);
        }

        // TODO: if no rendering, no video, ! generator_info.running().. -> terminate all
        // or just link renderer to generator in that case

        actor_info streamer_info{"streamer", streamer_};
        actor_info generator_info{"generator", generator};
        actor_info renderer_info{"renderer", renderer_};
        actor_info stdin_reader_info{"stdin_reader", stdin_reader_};

        while (streamer_info.running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            if (rendering_enabled) {
                s->send<message_priority::high>(renderer_, show_stats_v);
            } else {
                s->send<message_priority::high>(generator, show_stats_v);
            }
//        s->send<message_priority::high>(streamer_, debug_v);
//        s->send<message_priority::high>(generator, debug_v);
//        s->send<message_priority::high>(renderer_, debug_v);
//        if (use_stdin) {
//            s->send<message_priority::high>(stdin_reader_, debug_v);
//        }
        }
        if (renderer_info.running())
            s->send<message_priority::high>(renderer_, terminate__v);
        if (generator_info.running())
            s->send<message_priority::high>(generator, terminate__v);
        s->await_all_other_actors_done();
        starcry_running = false;
    }

    bool extract_host_port_string(string host_port_str, string *host_ptr, int *port_ptr)
    {
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

    //spawn_actor_local_or_remote("streamer", "use-remote-streamer", "127.0.0.1:11111")
    template <typename T>
    actor spawn_actor_local_or_remote(T * actor_behavior,
                                      string actor_name,
                                      string cli_flag_param,
                                      string cli_flag_value
    ){
        std::optional<actor> actor_ptr;
        if (vm.count(cli_flag_param)) {
            string host;
            int port = 0;
            if (!extract_host_port_string(cli_flag_value, &host, &port)) {
                cerr << "parameter for --" << cli_flag_param << " is invalid, please specify <host>:<port>, "
                     << "i.e. 127.0.0.1:11111." << endl;
            }
            cout << "using remote " << actor_name << " at: " << host << ":" << port << endl;
            auto p = system.middleman().remote_actor(host, port);
            if (!p) {
                cout << "connecting to " << actor_name << " failed: " << system.render(p.error()) << endl;
            }
            actor_ptr = *p;
        } else {
            std::optional<size_t> no_port;
            cout << "spawning local " << actor_name << endl;
            actor_ptr = system.spawn<caf::spawn_options::priority_aware_flag>(actor_behavior, no_port);
        }
        return *actor_ptr;
    }
};

int main(int argc, char *argv[]) {
    caf::exec_main_init_meta_objects<caf::id_block::starcry, io::middleman>();
    caf::core::init_global_meta_objects();
    actor_system_config cfg;
    cfg.load<io::middleman>();
    actor_system system(cfg);
    main_program prog{system, argc, argv};
    return 0;
}

#endif
#endif
