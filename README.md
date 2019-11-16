# ExpressCpp

Fast, unopinionated, minimalist web framework for C++
Perfect for building REST APIs

![Logo of ExpressCpp](./doc/logo_expresscpp.png)


[![Conan](https://api.bintray.com/packages/expresscpp/expresscpp/expresscpp%3Aexpresscpp/images/download.svg)](https://bintray.com/expresscpp/expresscpp/expresscpp%3Aexpresscpp/_latestVersion)
[![pipeline status](https://gitlab.com/expresscpp/expresscpp/badges/master/pipeline.svg)](https://gitlab.com/expresscpp/expresscpp/commits/master)
[![expresscpp_http](https://gitlab.com/expresscpp/expresscpp/badges/master/coverage.svg?job=test:linux:gcc9)](https://gitlab.com/expresscpp/expresscpp/commits/)
[![Status](https://img.shields.io/badge/quality-alpha-red)](https://img.shields.io/badge/quality-alpha-red)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![c++17](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)


## Design goals

ExpressCpp aims to be for C++ the same as express for Node.JS including its ecosystem of middlewares and extensions.

Express and Node.JS:

```js
const express = require('express');
const app = express();
app.get('/', (req, res) => res.send('Hello World!'));
const port = 3000;
app.listen(port, () => console.log(`Example app listening on port ${port}!`));
```

ExpressCpp:

```cpp
auto expresscpp = std::make_shared<ExpressCpp>();
expresscpp->Get("/", [](auto req, auto res) { res->Send("hello world!") });
const uint16_t port = 3000u;
expresscpp.Listen(port, [](){
    std::cout << "Example app listening on port "<< port << std::endl;
}).Run();
```

## Using me

### conan

```bash
conan remote add expresscpp https://api.bintray.com/conan/expresscpp/expresscpp/
```

add this to you conan file:

```txt
expresscpp/0.1.0@expresscpp/testing
```

this to your cmake:

```cmake
find_package(expresscpp)
# ...
target_link_libraries(my_target PRIVATE expresscpp::expresscpp)
```

### vendoring as subdirectory

```cmake
add_subdirectory(thirdparty/expresscpp)
# ...
target_link_libraries(my_target PRIVATE expresscpp::expresscpp)
```

## Build instructions (e.g. ubuntu)

### Dependencies

- boost/beast
- boost/asio
- nlohmann/json
- gtest (optional)
- openSSL (optional)

```bash
sudo apt install -y cmake gcc-9 g++-9 python3-pip

# we use conan for dependency management
sudo pip3 install conan --upgrade
conan remote add bincrafters "https://api.bintray.com/conan/bincrafters/public-conan"

# we use gcovr for coverage reports
sudo pip3 install gcovr --upgrade

mkdir -p build
cd build
cmake ..
make -j
```

## Examples


| name                 | file                                                                         |
|----------------------|------------------------------------------------------------------------------|
| query params         | [./example/query_params.cpp](./example/query_params.cpp)                     |
| url params           | [./example/url_params.cpp](./example/url_params.cpp)                         |
| auth-like middleware | [./example/middleware_auth_like.cpp](./example/middleware_auth_like.cpp)     |
| log-like middleware  | [./example/middleware_logger_like.cpp](./example/middleware_logger_like.cpp) |
| error handler        | [./example/error_handler.cpp](./example/error_handler.cpp)                   |
| variadic middlewares | [./example/multiple_handlers.cpp](./example/multiple_handlers.cpp)           |
| subrouting           | [./example/router.cpp](./example/router.cpp)                                 |


## Official Middlewares

- expresscpp-logger -> TODO
- expresscpp-static-files -> TODO
- expresscpp-grpc-proxy -> TODO
- expresscpp-reverse-proxy -> TODO
- expresscpp-basic-auth -> TODO


## Similiar projects

| name              | repo                                         | state        |
|-------------------|----------------------------------------------|--------------|
| BeastHttp         | https://github.com/0xdead4ead/BeastHttp/     |              |
| crow              | https://github.com/ipkn/crow                 | unmaintained |
| Simple-Web-Server | https://gitlab.com/eidheim/Simple-Web-Server |              |
| restinio          | https://github.com/stiffstream/restinio      |              |
| served            | https://github.com/meltwater/served          |              |
