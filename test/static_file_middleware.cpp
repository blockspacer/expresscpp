#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include "expresscpp/expresscpp.hpp"
#include "test_utils.hpp"

TEST(StaticFileMiddleware, ServeIndexHtml) {
  auto expresscpp = std::make_shared<ExpressCpp>();

  std::string index_html_content =
      R"(<!doctype html>
<html>
  <head>
    <title>This is the title of the webpage!</title>
  </head>
  <body>
    <p>This is a paragraph.</p>
  </body>
</html>
)";

  std::filesystem::path path_to_index_html = "/tmp/index.html";

  std::ofstream index_html_file(path_to_index_html);

  index_html_file << index_html_content << std::endl;

  index_html_file.close();

  assert(std::filesystem::exists(path_to_index_html));

  // ExpressJS: app.use(express.static('tmp'))
  expresscpp->Use(expresscpp->GetStaticFileProvider(path_to_index_html.parent_path()));

  expresscpp->Listen(8081, [=]() {
    // should get the index.html file
    auto index_html_contect_response = getResponse("/", boost::beast::http::verb::get);

    index_html_contect_response.erase(
        std::remove(index_html_contect_response.begin(), index_html_contect_response.end(), '\n'),
        index_html_contect_response.end());
    std::string index_html_content_expected = index_html_content;
    index_html_content_expected.erase(
        std::remove(index_html_content_expected.begin(), index_html_content_expected.end(), '\n'),
        index_html_content_expected.end());

    EXPECT_EQ(index_html_contect_response, index_html_content_expected);
  });
}