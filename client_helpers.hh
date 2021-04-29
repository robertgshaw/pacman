#include <string>
#include "nlohmann/json.hpp"

// This file contains code which implements some helpers to handle the APIs
//      used to communicate between the client and the server

// std::string format_client request(json request)
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xxx, body=abcdef...

std::string format_client_request(nlohmann::json request);