#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "BpBase.h"
#include <string>
#include <workflow/WFTaskFactory.h>
#include <nlohmann/json.hpp>
#include <vector>

using namespace nlohmann;
using std::string;
using std::vector;

class HttpClient : BpBase{
public:
    string url;

    int redirect_max;
    int retry_max;

    HttpClient(const string& url);
    void GET(const vector<std::pair<string, string>>& headers, http_callback_t callback);
    void POST(const vector<std::pair<string, string>>& headers, const json& body, http_callback_t callback);
};

#endif