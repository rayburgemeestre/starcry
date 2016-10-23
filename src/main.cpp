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

using start         = atom_constant<atom("start     ")>;
using show_stats    = atom_constant<atom("show_stats")>;
using debug         = atom_constant<atom("debug     ")>;
using terminate_    = atom_constant<atom("terminate ")>;
using initialize    = atom_constant<atom("initialize")>;

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
    string output_file            = "output.h264";
    uint32_t settings_            = 0;
    po::options_description desc  = string("Allowed options");
    string script                 = "test.js";
    bool compress                 = false;
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

        settings conf;
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
            ("stream", "start embedded webserver, crtmpserver and stream to it on local host")
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
        shared_ptr<webserver> ws;
        if (vm.count("stream") || vm.count("webserver")) {
            ws = make_shared<webserver>();
        }
        shared_ptr<crtmpserver_wrapper> cw;
        if (vm.count("stream") || vm.count("crtmpserver")) {
            cw = make_shared<crtmpserver_wrapper>();;
        }
        if (vm.count("stream")) {
            if (output_file.substr(0, 4) != "rtmp") {
                output_file.assign("rtmp://localhost/flvplayback/video");
            }
        }

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

        s->request(generator, infinite, initialize::value).receive(
            [&](size_t bitrate, bool use_stdin_, size_t use_fps_) {
//                const auto &bitrate = std::get<0>(tpl);
//                const auto &use_stdin_ = std::get<1>(tpl);
                use_stdin = use_stdin_;
                use_fps   = use_fps_;
                s->send(streamer_, initialize::value, int(conf.user.gui_port), string(output_file), bitrate, use_fps, output_settings);
            },
            [=](error &err) {
                std::exit(2);
            }
        );
        s->send(renderer_, initialize::value, streamer_, generator, workers_vec, streamer_host, streamer_port);

        s->send(generator, start::value, max_jobs_queued_for_renderer, num_chunks, renderer_);
        s->send(renderer_, start::value, use_remote_workers ? workers_vec.size() : num_workers);

        if (use_stdin) {
            s->send(stdin_reader_, start::value);
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
                s->send<message_priority::high>(renderer_, show_stats::value);
            } else {
                s->send<message_priority::high>(generator, show_stats::value);
            }
//        s->send<message_priority::high>(streamer_, debug::value);
//        s->send<message_priority::high>(generator, debug::value);
//        s->send<message_priority::high>(renderer_, debug::value);
//        if (use_stdin) {
//            s->send<message_priority::high>(stdin_reader_, debug::value);
//        }
        }
        if (renderer_info.running())
            s->send<message_priority::high>(renderer_, terminate_::value);
        if (generator_info.running())
            s->send<message_priority::high>(generator, terminate_::value);
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
    actor spawn_actor_local_or_remote(auto * actor_behavior,
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
            actor_ptr = system.spawn<priority_aware>(actor_behavior, no_port);
        }
        return *actor_ptr;
    }
};

int main(int argc, char *argv[]) {
    actor_system_config cfg;
    //cfg.scheduler_enable_profiling = true;
    //cfg.scheduler_profiling_ms_resolution = 100;
    //cfg.scheduler_profiling_output_file = "/projects/starcry/output_stats";
    cfg.scheduler_max_throughput = 1000;
    cfg.add_message_type<data::job>("data::job");
    cfg.add_message_type<data::pixel_data>("data::pixel_data");
    cfg.add_message_type<data::pixel_data2>("data::pixel_data2");
    cfg.add_message_type<vector<uint32_t>>("vector<uint32_t>");
    cfg.add_message_type<vector<pair<string, int>>>("workers_vec");
    cfg.load<io::middleman>();
    actor_system system(cfg);
    //auto max_thoughput_per_run = 1000;
    //set_scheduler(std::thread::hardware_concurrency(), max_thoughput_per_run);
    main_program prog{system, argc, argv};
    return 0;
}

#endif
