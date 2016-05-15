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

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>


using start         = atom_constant<atom("start     ")>;
using input_line    = atom_constant<atom("input_line")>;
using no_more_input    = atom_constant<atom("no_more_in")>;
using show_stats    = atom_constant<atom("show_stats")>;

#include <regex>
#include <bitset>
#include "announce.h"

//#include "caf/io/max_msg_size.hpp"
//caf::io::max_msg_size(std::numeric_limits<uint32_t>::max());

#include "caf/io/remote_actor.hpp"

int main(int argc, char *argv[]) {

    data::announce();

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
        try {
            auto client = io::remote_actor("127.0.0.1", conf.user.gui_port);
        }
        catch (network_error &err) {
            if (0 != system((std::string(argv[0]) + " --spawn-gui &").c_str())) {
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
        scoped_actor s;
        auto w = spawn(render_window, conf.user.gui_port);
        s->await_all_other_actors_done();
        return 0;
    }

    bool use_remote_workers    = vm.count("remote") || vm.count("worker");
    int range_begin = 0, range_end = 0;
    if (use_remote_workers) {
        regex range("([0-9]+)-([0-9]+)");
        smatch m;
        if (std::regex_match(worker_ports, m, range) && m.size() == 3) {
            { stringstream os(m[1]); os >> range_begin; }
            { stringstream os(m[2]); os >> range_end; }
            if (range_end < range_begin) swap(range_begin, range_end);
            num_workers = range_end - (range_begin - 1); // overruling
        }
    }

    if (use_remote_workers) cout << "use_remote_workers=true" << endl;
    if (worker_port) cout << "worker_port=" << worker_port << endl;
    if (num_chunks) cout << "num_chunks=" << num_chunks << endl;
    if (num_workers) cout << "num_workers=" << num_workers << endl;

    if (use_remote_workers && worker_port) {
        scoped_actor s;
        auto w = spawn(worker, worker_port, use_remote_workers);
        s->await_all_other_actors_done();
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

    scoped_actor s;
    auto jobstorage = spawn(job_storage);
    auto generator  = spawn<detached>(job_generator, jobstorage, script, canvas_w, canvas_h, use_stdin);
    auto streamer_  = spawn<priority_aware>(streamer, jobstorage, conf.user.gui_port, output_file, streamer_settings.to_ulong());
    auto renderer_  = spawn(renderer, jobstorage, streamer_, range_begin, range_end);

    // cascade exit from renderer -> job generator -> job storage
    generator->link_to(renderer_);
    jobstorage->link_to(generator);
    streamer_->link_to(renderer_);

    actor_info renderer_info{renderer_};
    s->send(generator, start::value, num_chunks);
    s->send(renderer_, start::value, num_workers);
    s->send(streamer_, start::value, renderer_);

    if (use_stdin) {
        auto stdin_reader_ = spawn(stdin_reader, generator, jobstorage);
        stdin_reader_->link_to(renderer_);
        s->send(stdin_reader_, start::value);
    }

    while (renderer_info.running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        //s->send(renderer_, show_stats::value);
        s->send(message_priority::high, streamer_, show_stats::value);
    }
    s->await_all_other_actors_done();
}
