#include "expresscpp/impl/session.hpp"

#include "expresscpp/expresscpp.hpp"

Session::Session(tcp::socket &&socket, ExpressCpp *express_cpp)
    : stream_(std::move(socket)), lambda_(*this), express_cpp_(express_cpp) {
  assert(express_cpp_ != nullptr);
}

void Session::run() { do_read(); }

void Session::do_read() {
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  req_ = {};

  // Set the timeout.
  stream_.expires_after(std::chrono::seconds(30));

  // Read a request
  http::async_read(
      stream_, buffer_, req_,
      beast::bind_front_handler(&Session::on_read, shared_from_this()));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  // This means they closed the connection
  if (ec == http::error::end_of_stream) {
    return do_close();
  }

  if (ec) {
    return fail(ec, "read");
  }

  const auto path = std::string(req_.target());
  const auto method = beastVerbToHttpMethod(req_.method());
  const auto keep_alive = req_.keep_alive();

  auto req = std::make_shared<Request>(path, method);
  auto res = std::make_shared<Response>(this);
  res->KeepAlive(keep_alive);

  express_cpp_->HandleRequest(req, res);
}

void Session::on_write(bool close, beast::error_code ec,
                       std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) {
    return fail(ec, "write");
  }

  if (close) {
    // This means we should close the connection, usually because
    // the response indicated the "Connection: close" semantic.
    return do_close();
  }

  // We're done with the response so delete it
  res_ = nullptr;

  // Read another request
  do_read();
}

void Session::do_close() {
  // Send a TCP shutdown
  beast::error_code ec;
  stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

  // At this point the connection is closed gracefully
}