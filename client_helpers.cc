#include "client_helpers.hh"

static const char* request_header = "REQUEST len=";
static const char* body_header =", body=";

// std::string format_client request(json request)
//      wraps the client request in the API wrapper
//      will be of form REQUEST len=xxx, body=abcdef...

std::string format_client_request(nlohmann::json request) {
    // extract the body into string form
    std::string body_str = request.dump();

    // create the message to send
    std::string msg = request_header;               // REQUEST len=
    msg.append(std::to_string(body_str.size()));    // REQUEST len=xxx
    msg.append(body_header);                        // REQUEST len=xxx, body=
    msg.append(body_str);                           // REQUEST len=xxx, body=abcdef...

    return msg;
}
