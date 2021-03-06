#include "expresscpp/console.hpp"
#include "expresscpp/expresscpp.hpp"
#include "expresscpp/fetch.hpp"
#include "expresscpp/impl/routing_stack.hpp"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"
#include "test_utils.hpp"

using namespace expresscpp;
using namespace std::string_literals;

constexpr std::size_t port = 8081u;

std::shared_ptr<Response> RoutePath(ExpressCpp &app, std::string_view path, std::string_view test_data,
                                    HttpMethod method) {
  auto req = std::make_shared<Request>(path, method);
  auto res = std::make_shared<Response>(nullptr);
  std::string name = "special_header";
  req->setHeader(name, test_data.data());
  app.HandleRequest(req, res, nullptr);
  return res;
}

TEST(BasicTests, RunAndStop) {
  ExpressCpp expresscpp;
  std::thread runner([&expresscpp]() { expresscpp.Run(); });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  expresscpp.Stop();
  runner.join();
}

TEST(BasicTests, DumpStack) {
  {
    ExpressCpp app;
    app.Get("/", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/"); });
    const auto rs = app.Stack();
    auto contains = [&]() {
      for (const auto &r : rs) {
        Console::Debug(fmt::format(R"(contains "{}")", r.path));
        if (r.path == "/") {
          return true;
        }
      }
      return false;
    }();
    EXPECT_TRUE(contains);
  }
  {
    ExpressCpp app;
    app.Get("/a", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/"); });
    app.Get("/b", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/"); });
    const auto rs = app.Stack();
    auto contains_a = [&]() {
      for (const auto &r : rs) {
        if (r.path == "/a") {
          EXPECT_EQ(r.method, HttpMethod::Get);
          return true;
        }
      }
      return false;
    }();
    EXPECT_TRUE(contains_a);
    auto contains_b = [&]() {
      for (const auto &r : rs) {
        if (r.path == "/b") {
          EXPECT_EQ(r.method, HttpMethod::Get);
          return true;
        }
      }
      return false;
    }();
    EXPECT_TRUE(contains_b);
  }
}

TEST(BasicTests, DumpStackWithMiddleware) {
  ExpressCpp app;
  auto logger = [](auto /*req*/, auto /*res*/, auto next) {
    Console::Debug("function called");
    next();
  };
  app.Use(logger);

  app.Get("/", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/"); });
  const auto rs = app.Stack();
  std::size_t number_of_root_routes = 0;

  auto contains_routes_in_stack = [&]() {
    for (const auto &r : rs) {
      if (r.path == "/") {
        number_of_root_routes++;
        // logger has method all, route has method get
        EXPECT_TRUE(r.method == HttpMethod::Get || r.method == HttpMethod::All);
      }
    }

    if (number_of_root_routes == 2) {
      return true;
    }

    return false;
  }();

  EXPECT_TRUE(contains_routes_in_stack);
}

TEST(BasicTests, SingleRouteWithParams) {
  ExpressCpp app;
  app.Get("/:id", [](auto req, auto res, auto /*next*/) {
    EXPECT_EQ(req->GetParams().size(), 1);
    EXPECT_EQ(req->GetParams().at("id"), "10");
    res->Json(R"({"status": 1 })");
  });
  app.Listen(port, [=](auto ec) {
    EXPECT_FALSE(ec);
    auto r = fetch(fmt::format("localhost:{}/10", port), {.method = HttpMethod::Get});
    const auto expected = nlohmann::json::parse(r);
    EXPECT_EQ(expected["status"], 1);
  });
}

TEST(BasicTests, SingleRouteWithRangeParams) {
  ExpressCpp app;
  app.Get("/things/:from-:to", [](auto req, auto res, auto /*next*/) {
    EXPECT_EQ(req->GetParams().size(), 2);
    EXPECT_EQ(req->GetParams().at("from"), "157");
    EXPECT_EQ(req->GetParams().at("to"), "2158");
    res->Json(R"({"status": 1 })");
  });
  app.Listen(8081, [](auto ec) {
    EXPECT_FALSE(ec);
    auto r = fetch(fmt::format("http://localhost:{}/things/157-2158", port), {.method = HttpMethod::Get});
    const auto expected = nlohmann::json::parse(r);
    EXPECT_EQ(expected["status"], 1);
  });
}

TEST(BasicTests, SingleRouteWithParamsAndQueryParams) {
  ExpressCpp app;
  app.Get("/things/:id", [](auto req, auto res, auto /*next*/) {
    EXPECT_EQ(req->GetParams().size(), 1);
    EXPECT_EQ(req->GetParams().at("id"), "1234");
    EXPECT_EQ(req->GetQueryParams().size(), 1);
    EXPECT_EQ(req->GetQueryParams().at("key1"), "value1");
    res->Json(R"({"status": 1 })");
  });
  app.Listen(8081, [](auto ec) {
    EXPECT_FALSE(ec);
    auto r = fetch(fmt::format("http://localhost:{}/things/1234?key1=value1", port), {.method = HttpMethod::Get});
    const auto expected = nlohmann::json::parse(r);
    EXPECT_EQ(expected["status"], 1);
  });
}

TEST(BasicTests, SingleRouteWithQueryParams) {
  ExpressCpp app;
  app.Get("/", [](auto req, auto res, auto /*next*/) {
    EXPECT_EQ(req->GetParams().size(), 0);
    EXPECT_EQ(req->GetQueryParams().size(), 1);
    EXPECT_EQ(req->GetQueryParams().at("key1"), "value1");
    res->Json(R"({"status": 1 })");
  });
  app.Listen(8081, [](auto ec) {
    EXPECT_FALSE(ec);
    auto r = fetch(fmt::format("http://localhost:{}/?key1=value1", port), {.method = HttpMethod::Get});
    const auto expected = nlohmann::json::parse(r);
    EXPECT_EQ(expected["status"], 1);
  });
}

TEST(BasicTests, SingleRouteJson) {
  TestCallSleeper sleeper(1);
  ExpressCpp app;
  app.Get("/", [&](auto /*req*/, auto res, auto /*next*/) {
    sleeper.Call();
    res->Json(R"({"status": 1 })");
  });
  app.Listen(port, [](auto ec) {
    EXPECT_FALSE(ec);
    auto r = fetch(fmt::format("localhost:{}/", port), {.method = HttpMethod::Get});
    const auto expected = nlohmann::json::parse(r);
    EXPECT_EQ(expected["status"], 1);
  });
  EXPECT_TRUE(sleeper.Wait());
}

TEST(BasicTests, SingleRouteWithoutBeast) {
  {
    ExpressCpp app;

    app.Get("/", [](auto req, auto res, auto /*next*/) {
      auto headers = req->getHeaders();
      EXPECT_NE(headers.find("special_header"), headers.end());

      auto value = headers.at("special_header");
      EXPECT_EQ(value, "special_header_value");

      EXPECT_ANY_THROW(res->Json(R"({"status": 1 })"););
    });

    auto res = RoutePath(app, "/", "special_header_value", HttpMethod::Get);
    EXPECT_EQ(res->response_sent_, true);
  }

  {
    ExpressCpp app;

    app.Get("/aaa", [](auto req, auto res, auto /*next*/) {
      auto headers = req->getHeaders();
      EXPECT_NE(headers.find("special_header"), headers.end());

      auto value = headers.at("special_header");
      EXPECT_EQ(value, "special_header_value");

      EXPECT_ANY_THROW(res->Json(R"({"status": 1 })"););
    });

    auto res = RoutePath(app, "/aaa", "special_header_value", HttpMethod::Get);
    EXPECT_EQ(res->response_sent_, true);
  }

  {
    ExpressCpp app;

    app.Post("/aaa", [](auto req, auto res, auto /*next*/) {
      auto headers = req->getHeaders();
      EXPECT_NE(headers.find("special_header"), headers.end());

      auto value = headers.at("special_header");
      EXPECT_EQ(value, "special_header_value");

      EXPECT_ANY_THROW(res->Json(R"({"status": 1 })"););
    });

    auto res = RoutePath(app, "/aaa", "special_header_value", HttpMethod::Post);
    EXPECT_EQ(res->response_sent_, true);
  }
}

TEST(BasicTests, PostRouteWithoutBeast) {
  ExpressCpp app;

  app.Post("/aaa", [](auto req, auto res, auto /*next*/) {
    auto headers = req->getHeaders();
    EXPECT_NE(headers.find("special_header"), headers.end());

    auto value = headers.at("special_header");
    EXPECT_EQ(value, "special_header_value");

    EXPECT_ANY_THROW(res->Json(R"({"status": 1 })"););
  });

  auto res = RoutePath(app, "/aaa", "special_header_value", HttpMethod::Post);
  EXPECT_EQ(res->response_sent_, true);
}

TEST(BasicTests, MultiRoute) {
  ExpressCpp app;
  app.Get("/", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/"); });
  app.Get("/b", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/b"); });
  app.Listen(8081, [](auto ec) {
    EXPECT_FALSE(ec);
    auto r = fetch(fmt::format("localhost:{}/", port), {.method = HttpMethod::Get});
    auto rb = fetch(fmt::format("localhost:{}/b", port), {.method = HttpMethod::Get});
    EXPECT_EQ(r, "/");
    EXPECT_EQ(rb, "/b");
  });
}

TEST(BasicTests, MultipleListenCalls) {
  ExpressCpp app;
  app.Get("/a", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/a"); });
  app.Listen(port, [](const auto ec) { EXPECT_EQ(ec.value(), 0); });
  app.Listen(port, [](const auto ec) { EXPECT_EQ(ec, std::errc::already_connected); });
}

TEST(BasicTests, StartMultipleApps) {
  {
    ExpressCpp app1;
    constexpr std::size_t port1 = 8081u;
    app1.Get("/a", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/1"); });
    app1.Listen(port1, [&](std::error_code ec) {
      EXPECT_EQ(ec.value(), 0);
      auto r1 = fetch(fmt::format("localhost:{}/a", port1), {.method = HttpMethod::Get});
      EXPECT_EQ(r1, "/1");
    });

    ExpressCpp app2;
    constexpr std::size_t port2 = 8082u;
    app2.Get("/a", [](auto /*req*/, auto res, auto /*next*/) { res->Send("/2"); });
    app2.Listen(port2, [&](std::error_code ec) {
      EXPECT_EQ(ec.value(), 0);
      auto r2 = fetch(fmt::format("localhost:{}/a", port2), {.method = HttpMethod::Get});
      EXPECT_EQ(r2, "/2");
    });
  }
}
