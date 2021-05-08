#include "utilities.hh"

std::string parse_message(char* server_msg, const char* format, const char* b_format, int b_kw_len) {
    
	std::string event_str;
	int len, nread;
	char equals_sign;
    
    // parse string
    if(sscanf(server_msg, format, &len, &equals_sign) == 2 && equals_sign == '=') {
        event_str = server_msg;
        nread = event_str.size();           
        int body_start_ind = event_str.find(b_format) + b_kw_len;    
        int body_len_in_buf = std::min(len, nread - body_start_ind);
        event_str = event_str.substr(body_start_ind, body_len_in_buf);
    }

    return event_str;
}