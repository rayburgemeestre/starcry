#include "webserver.h"

/**
 * Similar to the fix in main.cpp, crow.h was also pulling in boost::shared_ptr, via boost::date_time stuff.
 * /opt/cppse/build/boost/include/boost/smart_ptr/detail/sp_counted_base_clang.hpp:29:9: warning: '_Atomic' is a C11 extension [-Wc11-extensions]
 */
#ifdef __clang__
#  pragma clang diagnostic ignored "-Wc11-extensions"
#endif // __clang__

#include <fstream>
#include <sstream>
#include "crow.h"
#include "json.h"

struct content_type_fixer {
  struct context {};

  void before_handle(crow::request& req, crow::response& res, context& ctx) {}

  void after_handle(crow::request& req, crow::response& res, context& ctx) {
    if (req.url.find("/images") == 0)
      res.set_header("Content-Type", "image/png");
    else if (req.url.find("/scripts") == 0)
      res.set_header("Content-Type", "text/javascript");
    else
      res.set_header("Content-Type", "text/html");

    if (req.url.find(".ts") != std::string::npos) {
      res.set_header("Content-Type", "video/mp2t");
    }
    res.set_header("Access-Control-Allow-Origin", "*");
  }
};

void start_webserver() {
  crow::App<content_type_fixer> app;

  CROW_ROUTE(app, "/api")
  ([]() { return "API call"; });

  CROW_ROUTE(app, "/api/test")
  ([]() {
    crow::json::wvalue x;
    std::string label{"A"};
    for (int i = 0; i < 6; i++) {
      x["data"][i]["label"] = label;
      x["data"][i]["value"] = 100;
      label[0]++;
    }
    return x;
  });

  CROW_ROUTE(app, "/")
  ([] {
    using namespace std;
    ifstream ifile("webroot/index.html", ios::binary);
    string s((istreambuf_iterator<char>(ifile)), (istreambuf_iterator<char>()));
    return s;
  });

  CROW_ROUTE(app, "/<string>")
  ([](std::string str) {
    using namespace std;
    ifstream ifile("webroot/"s + str, ios::binary);
    string s((istreambuf_iterator<char>(ifile)), (istreambuf_iterator<char>()));
    return s;
  });

  CROW_ROUTE(app, "/<string>/<string>")
  ([](std::string folder, std::string file) {
    using namespace std;
    ifstream ifile("webroot/"s + folder + "/" + file, ios::binary);
    string s((istreambuf_iterator<char>(ifile)), (istreambuf_iterator<char>()));
    // cout << "read = " << s.size() << " & " << s.length() << endl;
    // cout << "console = " << s.data() << endl;
    return s;
  });

  crow::logger::setLogLevel(crow::LogLevel::WARNING);

  app.port(18080).multithreaded().run();
}

webserver::webserver() : webserver_(start_webserver) {}

webserver::~webserver() {
  webserver_.join();
}
