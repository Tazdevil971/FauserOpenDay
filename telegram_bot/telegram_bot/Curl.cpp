#include "Curl.hpp"

#include <iostream>

Curl::Curl() :
	m_curl(curl_easy_init()) {}

Curl::Curl(Curl&& curl) :
	m_curl(curl.m_curl) {}

Curl::~Curl() {
	curl_easy_cleanup(m_curl);
}

bool Curl::request(std::string url, nlohmann::json& result) {
	curl_easy_setopt(m_curl, CURLOPT_POST, 0);
	curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, NULL);

	return requestCommon(url, result);
}

bool Curl::request(std::string url, nlohmann::json& result, const nlohmann::json parameters) {
	std::string parametersString = parameters.dump();
	curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");

	curl_easy_setopt(m_curl, CURLOPT_POST, 1);
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, parametersString.c_str());
	curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);

	return requestCommon(url, result);
}

bool Curl::requestCommon(std::string url, nlohmann::json& result) {
	std::string resultString;

	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &resultString);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, Curl::writeFunc);

	if(curl_easy_perform(m_curl) != CURLE_OK)
		return false;

	try {
		result = nlohmann::json::parse(resultString);
	} catch(std::exception&) {
		return false;
	}
	
	return true;
}

size_t Curl::writeFunc(char* ptr, size_t size, size_t nmemb, void* userdata) {
	size_t actualData = size * nmemb;
	std::string* resultString = reinterpret_cast<std::string*>(userdata);

	resultString->append(ptr, actualData);

	return actualData;
}