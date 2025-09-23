#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <workflow/WFTaskFactory.h>
#include <string>

#include <vector>

using std::string;
using std::vector;

class HttpClient {
public:
    string url;
    http_callback_t callback;

    int redirect_max;
    int retry_max;
    enum method {
        GET,
        POST,
        PUT,
        DELETE,
        HEAD,
        OPTIONS,
        PATCH
    };

    HttpClient(string url, vector<std::pair<string, string>> headers,) {
        this->url = url;
    }
};

#endif