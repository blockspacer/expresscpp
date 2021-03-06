#include "expresscpp/console.hpp"
#include "expresscpp/expresscpp.hpp"

using namespace expresscpp;

int main() {
  ExpressCpp app;

  app.Get("/things/:id/datapoint/:uuid", [](auto req, auto res, auto /*next*/) {
    std::cout << "params: " << std::endl;
    for (const auto& [name, value] : req->GetParams()) {
      std::cout << "    name: " << name << ", value: " << value << std::endl;
    }

    res->Json(R"({ "status": "ok" })");
  });

  constexpr uint16_t port = 8081;

  const auto output = fmt::format(
      R"(you can try now: "curl http://localhost:{}/things/1234/datapoint/080fc7a2-9128-4491-bc9d-df05a34064cb")",
      port);

  app.Listen(port,
             [=](auto ec) {
               if (ec) {
                 std::cerr << "ERROR: " << ec.message() << std::endl;
                 std::cerr << "exiting..." << std::endl;
                 exit(1);
               }
               std::cout << output << std::endl;
             })
      .Run();

  return 0;
}
