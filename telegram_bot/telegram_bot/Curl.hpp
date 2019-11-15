#pragma once

#include <curl/curl.h>
#include <nlohmann/json.hpp>

class Curl {
public:
	Curl();
	Curl(Curl&) = delete;
	Curl(Curl&& curl);

	~Curl();

	bool request(std::string url, nlohmann::json& result);
	bool request(std::string url, nlohmann::json& result, const nlohmann::json parameters);

private:
	bool requestCommon(std::string url, nlohmann::json& result);
	static size_t writeFunc(char* ptr, size_t size, size_t nmemb, void* userdata);

	CURL* m_curl;
};