#include "utilities.hh"

std::tuple<std::string, int> parse_message(char* server_msg, const char* format, const char* b_format, int b_kw_len) {
    
	std::string body_str;
	int len, nread, body_start_ind, body_len_in_buf;
	char equals_sign;
    
    // parse string
    if(sscanf(server_msg, format, &len, &equals_sign) == 2 && equals_sign == '=') {
        body_str = server_msg;
        nread = body_str.size();           
        body_start_ind = body_str.find(b_format) + b_kw_len;    
        body_len_in_buf = std::min(len, nread - body_start_ind);
        body_str = body_str.substr(body_start_ind, body_len_in_buf);
    }

    return std::make_tuple(body_str, len - body_len_in_buf);
}