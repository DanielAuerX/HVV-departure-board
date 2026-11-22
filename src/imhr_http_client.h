#pragma once

#include <Arduino.h>

namespace imhr
{
    struct HttpResponse
    {
        int code;
        String body;
        bool success() const { return code == 200; }
    };

    HttpResponse sendPostRequest(const String &url, const String &body);
    HttpResponse sendGetRequest(const String &url);
    String signRequest(const char *payload, const char *secret);
}
