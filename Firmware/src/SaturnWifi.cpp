#include "SaturnWifi.hpp"

#if (WIFI_ENABLED == 1)

#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WebServer.h>

SaturnWifi::SaturnWifi() {
    _webServer = static_cast<void*>(new WebServer(80));
    _fileSystemReady = false;
    _webServerReady = false;
}

void SaturnWifi::init() {
    // Placeholder for future initialisation hooks.
}

void SaturnWifi::mountFileSystem() {
    _fileSystemReady = LittleFS.begin(false);
    if (!_fileSystemReady) {
        Serial.println("LittleFS mount failed, attempting to format...");
        _fileSystemReady = LittleFS.begin(true);
    }
    if (_fileSystemReady) {
        Serial.printf("LittleFS mounted. Total: %u bytes, Used: %u bytes\n",
                      LittleFS.totalBytes(),
                      LittleFS.usedBytes());
    } else {
        Serial.println("LittleFS unavailable. Static file hosting disabled.");
    }
}

void SaturnWifi::loop() {
    if (!_webServerReady) {
        return;
    }
    auto *server = static_cast<WebServer*>(_webServer);
    server->handleClient();
}

String SaturnWifi::getContentType(const String &path) {
    if (path.endsWith(".html")) return "text/html";
    if (path.endsWith(".htm")) return "text/html";
    if (path.endsWith(".css")) return "text/css";
    if (path.endsWith(".js")) return "application/javascript";
    if (path.endsWith(".json")) return "application/json";
    if (path.endsWith(".ico")) return "image/x-icon";
    if (path.endsWith(".png")) return "image/png";
    if (path.endsWith(".jpg")) return "image/jpeg";
    if (path.endsWith(".jpeg")) return "image/jpeg";
    if (path.endsWith(".svg")) return "image/svg+xml";
    if (path.endsWith(".woff2")) return "font/woff2";
    if (path.endsWith(".txt")) return "text/plain";
    if (path.endsWith(".wasm")) return "application/wasm";
    return "application/octet-stream";
}

bool SaturnWifi::tryServeStaticFile(const String &requestedPath) {
    if (!_fileSystemReady) {
        return false;
    }

    String path = requestedPath;
    if (path.isEmpty() || path == "/") {
        path = "/index.html";
    }
    if (path.endsWith("/")) {
        path += "index.html";
    }

    bool isGzipped = false;
    if (!LittleFS.exists(path)) {
        String gzPath = path + ".gz";
        if (LittleFS.exists(gzPath)) {
            path = gzPath;
            isGzipped = true;
        } else {
            return false;
        }
    }

    File file = LittleFS.open(path, "r");
    if (!file || file.isDirectory()) {
        return false;
    }

    String contentType = getContentType(path);
    auto *server = static_cast<WebServer*>(_webServer);
    if (isGzipped) {
        contentType = getContentType(path.substring(0, path.length() - 3));
        server->sendHeader("Content-Encoding", "gzip");
    }

    server->sendHeader("Cache-Control", "public, max-age=900");
    server->streamFile(file, contentType);
    file.close();
    return true;
}

void SaturnWifi::send(int statusCode, const String &contentType, const String &content)
{
    auto *server = static_cast<WebServer*>(_webServer);
    server->send(statusCode, contentType, content);
}

void SaturnWifi::registerRoute(const String &path, SaturnHttpMethod method, const std::function<void(void)> &handler)
{
    auto *server = static_cast<WebServer*>(_webServer);
    server->on(path.c_str(), static_cast<HTTPMethod>(method), handler);
}

void SaturnWifi::registerRouteNotFound()
{
    auto *server = static_cast<WebServer*>(_webServer);
    server->onNotFound([this]() {
        auto *serverInner = static_cast<WebServer*>(this->_webServer);
        const String uri = serverInner->uri();
        if (uri.startsWith("/api/")) {
            serverInner->send(404, "application/json", "{\"error\":\"not found\"}");
            return;
        }
        if (this->tryServeStaticFile(uri)) {
            return;
        }
        if (this->tryServeStaticFile("/")) {
            return;
        }
        serverInner->send(404, "text/plain", "Not Found");
    });
}

bool SaturnWifi::connectToWiFi(const String &hostname, const String &ssid, const String &wpaKey)
{
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname.c_str());
    WiFi.begin(ssid.c_str(), wpaKey.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_STA_CONNECT_TIMEOUT_MS) {
        delay(200);
    }

    auto *server = static_cast<WebServer*>(_webServer);
    if (WiFi.status() == WL_CONNECTED) {
        _webServerReady = true;
        server->begin();
        IPAddress stationIP = WiFi.localIP();
        Serial.println("Connected to WiFi network");
        Serial.print("Station IP address: ");
        Serial.println(stationIP);
        return true;
    }

    Serial.println("Failed to connect to WiFi network");
    return false;
}
#endif
