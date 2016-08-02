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

    // TODO: apparently, I still need to announce :-)
    cfg.add_message_type<data::job>("data::job");
    cfg.add_message_type<data::pixel_data>("data::pixel_data");
    cfg.add_message_type<data::pixel_data2>("data::pixel_data2");
    cfg.add_message_type<vector<uint32_t>>("vector<uint32_t>");

    cfg.load<io::middleman>();

    actor_system system(cfg);
    auto w = system.spawn(remote_worker, atoi(argv[1]));
    return 0;
}
#else
#include <iostream>

#include "common.h"
#include "actors/job_storage.h"
#include "actors/job_generator.h"
#include "actors/renderer.h"
#include "actors/streamer.h"
#include "actors/render_window.h"
#include "actors/stdin_reader.h"

#include "util/actor_info.hpp"
#include "util/settings.hpp"

#include "webserver.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>


using start         = atom_constant<atom("start     ")>;
using input_line    = atom_constant<atom("input_line")>;
using no_more_input = atom_constant<atom("no_more_in")>;
using show_stats    = atom_constant<atom("show_stats")>;
using debug         = atom_constant<atom("debug     ")>;

#include <regex>
#include <bitset>

//#include "caf/io/max_msg_size.hpp"
//caf::io::max_msg_size(std::numeric_limits<uint32_t>::max());

#include "caf/io/middleman.hpp"

#include "data/pixels.hpp"
#include "data/job.hpp"

int main(int argc, char *argv[]) {
    actor_system_config cfg;
    //cfg.scheduler_enable_profiling = true;
    //cfg.scheduler_profiling_ms_resolution = 100;
    //cfg.scheduler_profiling_output_file = "/projects/starcry/output_stats";

    cfg.add_message_type<data::job>("data::job");
    cfg.add_message_type<data::pixel_data>("data::pixel_data");
    cfg.add_message_type<data::pixel_data2>("data::pixel_data2");
    cfg.add_message_type<vector<uint32_t>>("vector<uint32_t>");

    cfg.load<io::middleman>();

    actor_system system(cfg);

    //auto max_thoughput_per_run = 1000;
    //set_scheduler(std::thread::hardware_concurrency(), max_thoughput_per_run);

    settings conf;
    conf.load();

    namespace po = ::boost::program_options;
    int worker_port            = 0;
    size_t num_chunks          = 1; // number of chunks to split image size_to
    size_t num_workers         = 8; // number of workers for rendering
    string worker_ports;
    string dimensions;
    string output_file{"output.h264"};
    uint32_t settings_{0};
    bitset<32> streamer_settings(settings_);
    po::options_description desc{"Allowed options"};
    string script{"test.js"};
    bool rendering_enabled = true;

    po::positional_options_description p;
    p.add("script", 1);
    p.add("output", 1);

    desc.add_options()("help", "produce help message")
                      ("output,o", po::value<string>(&output_file), "filename for video output (default output.h264)")
                      ("remote,r", po::value<string>(&worker_ports), "use remote workers for rendering")
                      ("worker,w", po::value<int>(&worker_port), "start worker on specified port")
                      ("num-chunks,c", po::value<size_t>(&num_chunks), "number of jobs to split frame in (default 1)")
                      ("num-workers,n", po::value<size_t>(&num_workers), "number of workers to use in render pool (default 8)")
                      ("gui", "open GUI window (writes state to $HOME/.starcry.conf)")
                      ("spawn-gui", "spawn GUI window (used by --gui, you probably don't need to call this)")
                      ("no-video-output", "disable video output using ffmpeg")
                      ("no-rendering", "disable rendering (useful for testing javascript performance)")
                      ("webserver", "start embedded webserver")
                      ("stdin", "read from stdin and send this to job generator actor")
                      ("dimensions,dim", po::value<string>(&dimensions), "specify canvas dimensions i.e. 1920x1080")
                      ("script,s", po::value<string>(&script), "javascript file to use for processing")
        ;
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    try {
        po::notify(vm);
    }
    catch (po::error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        std::cout << desc << std::endl;
        return false;
    }
    rendering_enabled = ! vm.count("no-rendering");
    if (vm.count("script")) {
        cout << "Script: " << vm["script"].as<string>() << "\n";
    }
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    // determine outputs
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
    if (vm.count("spawn-gui")) {
        cout << "launching render output window" << endl;
        auto w = system.spawn(render_window, conf.user.gui_port);
        return 0;
    }

    bool use_remote_workers    = vm.count("remote") || vm.count("worker");
    vector<pair<string, int>> workers_vec;
    if (use_remote_workers) {
        ifstream infile(worker_ports);
        string line;
        while (getline(infile, line))
        {
            if (line[0] == ';' || line[0] == '#') continue;
            size_t pos = line.find(":");
            if (pos != string::npos)
                line[pos] = ' ';
            pos = line.find(" ");
            if (pos == string::npos) {
                cerr << "erroneous line in servers text: " << line << endl;
                continue;
            }
            cout << "found server: " << line.substr(0, pos) << " and port: " << atoi(line.substr(pos + 1).c_str()) << endl;
            workers_vec.push_back(make_pair(line.substr(0, pos), atoi(line.substr(pos + 1).c_str())));
        }
    }

    if (use_remote_workers) cout << "use_remote_workers=true" << endl;
    if (worker_port) cout << "worker_port=" << worker_port << endl;
    if (num_chunks) cout << "num_chunks=" << num_chunks << endl;
    if (num_workers) cout << "num_workers=" << num_workers << endl;

    if (use_remote_workers && worker_port) {
        auto w = system.spawn(remote_worker, worker_port);
        return 0;
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

    scoped_actor s{system};
    auto jobstorage = system.spawn(job_storage);
    auto generator  = system.spawn<priority_aware + detached>(job_generator, jobstorage, script, canvas_w, canvas_h, use_stdin, rendering_enabled);
    // generator links to job storage
    auto streamer_  = system.spawn<priority_aware>(streamer, jobstorage, conf.user.gui_port, output_file, streamer_settings.to_ulong());
    auto renderer_  = system.spawn(renderer, jobstorage, streamer_, workers_vec);
    // renderer links to streamer and job storage

    actor_info streamer_info{streamer_};
    s->send(generator, start::value, num_chunks);
    s->send(renderer_, start::value, use_remote_workers ? workers_vec.size() : num_workers);

    if (use_stdin) {
        auto stdin_reader_ = system.spawn(stdin_reader, generator, jobstorage);
        s->send(stdin_reader_, start::value);
    }

    shared_ptr<webserver> ws;
    if (vm.count("webserver")) {
        ws = make_shared<webserver>();
    }

    while (streamer_info.running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (rendering_enabled) {
            s->send<message_priority::high>(streamer_, show_stats::value);
        } else {
            s->send<message_priority::high>(generator, show_stats::value);
        }
//        s->send(streamer_, debug::value);
//        s->send(generator, debug::value);
//        s->send(jobstorage, debug::value);
//        s->send(renderer_, debug::value);
    }
    s->await_all_other_actors_done();
}
#endif
