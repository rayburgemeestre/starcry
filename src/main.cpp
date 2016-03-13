#include <iostream>

#include "common.h"
#include "job_storage.h"
#include "job_generator.h"
#include "renderer.h"
#include "streamer.h"
#include "render_window.h"

#include "util/actor_info.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>


using start         = atom_constant<atom("start     ")>;
using show_stats    = atom_constant<atom("show_stats")>;

#include <regex>
#include <bitset>
#include "announce.h"

//#include "caf/io/max_msg_size.hpp"
//caf::io::max_msg_size(std::numeric_limits<uint32_t>::max());

extern void initialize_v8_wrapper();
extern void deinitialize_v8_wrapper();

int main(int argc, char *argv[]) {

    initialize_v8_wrapper();

    data::announce();

    namespace po = ::boost::program_options;
    int worker_port            = 0;
    int render_win_port        = 0;
    int render_win_port_at     = 0;
    size_t num_chunks          = 8; // number of chunks to split image size_to
    size_t num_workers         = 8; // number of workers for rendering
    string worker_ports;
    string dimensions;
    uint32_t settings_{0};
    bitset<32> streamer_settings(settings_);
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")
                      ("remote,r", po::value<string>(&worker_ports), "use remote workers for rendering")
                      ("worker,w", po::value<int>(&worker_port), "start worker on specified port")
                      ("num-chunks,c", po::value<size_t>(&num_chunks), "number of jobs to split frame in")
                      ("num-workers,n", po::value<size_t>(&num_workers), "number of workers to use in render pool")
                      ("render-window", po::value<int>(&render_win_port), "launch a render window on specified port")
                      ("render-window-at", po::value<int>(&render_win_port_at), "use render window on specified port")
                      ("no-video-output", "disable video output using ffmpeg")
                      ("dimensions,dim", po::value<string>(&dimensions), "specify canvas dimensions i.e. 1920x1080")
        ;
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    try {
        po::notify(vm);
    }
    catch (po::error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        std::cout << desc << std::endl;
        return false;
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
    if (render_win_port_at) {
        cout << "Enabling video output to window on port " << render_win_port_at << ".." << endl;
        streamer_settings.set(streamer_allegro5, true);
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

    if (render_win_port) {
        cout << "launching render output window on port " << render_win_port << endl;
        scoped_actor s;
        auto w = spawn(render_window, render_win_port);
        s->await_all_other_actors_done();
        return 0;
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

    scoped_actor s;
    auto jobstorage = spawn(job_storage);
    auto generator  = spawn(job_generator, jobstorage, canvas_w, canvas_h);
    auto streamer_  = spawn(streamer, jobstorage, render_win_port_at, streamer_settings.to_ulong());
    auto renderer_  = spawn(renderer, jobstorage, streamer_, range_begin, range_end);

    // cascade exit from renderer -> job generator -> job storage
    generator->link_to(renderer_);
    jobstorage->link_to(generator);
    streamer_->link_to(renderer_);

    actor_info renderer_info{renderer_};
    s->send(generator, start::value, num_chunks);
    s->send(renderer_, start::value, num_workers);

    while (renderer_info.running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        //s->send(renderer_, show_stats::value);
        s->send(streamer_, show_stats::value);
    }
    s->await_all_other_actors_done();

    deinitialize_v8_wrapper();
}
