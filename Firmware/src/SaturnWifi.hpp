/*
  SaturnWifi.hpp - Wifi handler and Webserver abstraction
  Copyright (c) 2025 Lutz Kretzschmar.  All right reserved.

  Remarks:

*/

#ifndef SATURN_WIFI_HPP
#define SATURN_WIFI_HPP

#include <Arduino.h>
#include "../Configuration.hpp"

#if (WIFI_ENABLED == 1)

#include <functional>

class WebServer;

enum class SaturnHttpMethod : uint8_t {
    Any = 0,
    Get = 1,
    Post = 2,
    Put = 3,
    Patch = 4,
    Delete = 5,
    Options = 6
};

class SaturnWifi
{
    void *_webServer;
    bool _fileSystemReady;
    bool _webServerReady;
 public:
    SaturnWifi();

    void init();
    void loop();
    void mountFileSystem();
    String getContentType(const String &path);
    bool tryServeStaticFile(const String &requestedPath);

    void registerRoute(const String &path, SaturnHttpMethod method, const std::function<void(void)> &handler);
    void send(int statusCode, const String &contentType, const String &content);
    void registerRouteNotFound();
    bool connectToWiFi(const String &hostname, const String &ssid, const String &wpaKey);
};
#endif

#endif