#include <string>
#include <sys/socket.h> // socket
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <tuple>

std::tuple<std::string, int> parse_message(char* server_msg, const char* format, const char* b_format, int b_kw_len);