#include "http_message.hpp"

HttpRequest::HttpRequest(const std::string& request) {
	size_t method_end_pos = request.find(' ');
	std::string str_method = request.substr(0, method_end_pos);
	if (str_method == "GET")
		method = HTTP_METHOD_GET;
	else if(str_method == "PUT")
		method = HTTP_METHOD_PUT;
	else if (str_method == "POST")
		method = HTTP_METHOD_POST;

	size_t url_end_pos = request.find(' ', ++method_end_pos);
	url = request.substr(method_end_pos, url_end_pos - method_end_pos);
	//parse http version
	size_t version_end_pos = request.find("\r\n", ++url_end_pos);
	std::string version_str = request.substr(url_end_pos, version_end_pos - url_end_pos);
	if (version_str == HTTP_09_VERION_STR)
		http_version = HTTP_VERSION_09;
	if (version_str == HTTP_10_VERION_STR)
		http_version = HTTP_VERSION_10;
	else if (version_str == HTTP_11_VERION_STR)
		http_version = HTTP_VERSION_11;
	else if (version_str == HTTP_20_VERION_STR)
		http_version = HTTP_VERSION_20;
	//parse host
	size_t host_begin_pos = request.find("Host: ") + 6;
	if (host_begin_pos != std::string::npos) {
		size_t host_end_pos = request.find('\r\n', host_begin_pos);
		host = request.substr(host_begin_pos, host_end_pos - host_begin_pos);
	}
	//parse user agent
	size_t user_agent_begin_pos = request.find("User-Agent: ") + 12;
	if (user_agent_begin_pos != std::string::npos) {
		size_t user_agent_end_pos = request.find('\r\n', user_agent_begin_pos);
		if (user_agent_end_pos != std::string::npos)
			user_agent = request.substr(user_agent_begin_pos, user_agent_end_pos - user_agent_begin_pos);
	}
}

HttpVersion HttpRequest::getVersion() const {
	return http_version;
}

const std::string& HttpRequest::getUserAgent() const {
	return user_agent;
}

const std::string& HttpRequest::getUrl() const {
	return url;
}

HttpResponse::HttpResponse(
	HttpVersion version,
	int code,
	const std::string& content,
	const std::string& content_type
) :
	http_version(version),
	data_content(content),
	code(code),
	content_type(content_type)
{
}

void HttpResponse::Create() {
	switch (http_version) {
	case HTTP_VERSION_09: {
		content += HTTP_09_VERION_STR;
		break;
	}
	case HTTP_VERSION_10: {
		content += HTTP_10_VERION_STR;
		break;
	}
	case HTTP_VERSION_11: {
		content += HTTP_11_VERION_STR;
		break;
	}
	case HTTP_VERSION_20: {
		content += HTTP_20_VERION_STR;
		break;
	}
	}
	content += " ";
	content += std::to_string(code) + " OK\n";
	content += "Content-Type: " + content_type + "\n";
	content += "Content-Length: " + std::to_string(data_content.size()) + "\n";
	content += "Connection: Closed\n";
	content += "\n" + data_content;

}
const std::string& HttpResponse::getContent() {
	return content;
}