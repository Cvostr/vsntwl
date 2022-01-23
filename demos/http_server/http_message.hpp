#pragma once
#include <string>

#define HTTP_09_VERION_STR "HTTP/0.9"
#define HTTP_10_VERION_STR "HTTP/1.0"
#define HTTP_11_VERION_STR "HTTP/1.1"
#define HTTP_20_VERION_STR "HTTP/2.0"

enum HttpMethod {
	HTTP_METHOD_GET,
	HTTP_METHOD_PUT,
	HTTP_METHOD_POST
};

enum HttpVersion {
	HTTP_VERSION_09,
	HTTP_VERSION_10,
	HTTP_VERSION_11,
	HTTP_VERSION_20
};

class HttpRequest {
private:
	HttpMethod method;
	std::string url;
	std::string host;
	HttpVersion http_version;
	std::string user_agent;
	std::string content;
public:
	HttpRequest(const std::string& request);

	HttpVersion getVersion() const;
	const std::string& getUserAgent() const;
	const std::string& getUrl() const;
};

class HttpResponse {
private:
	std::string content;

	HttpVersion http_version;
	int code;
	std::string data_content;
	std::string content_type;
public:
	HttpResponse(HttpVersion version, int code, const std::string& content, const std::string& content_type = "text/html");

	void Create();
	const std::string& getContent();
};