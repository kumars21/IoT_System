#include <HTTP_Method.h>
#include <Middlewares.h>
#include <Uri.h>
#include <WebServer.h>

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include <Preferences.h>
#include <ArduinoJson.h>
#include "index_html.h"

static const char *hotspot_ssid = "SmartLock";
static const char *hotspot_password = nullptr;

WebServer server(80);
Preferences preferences;

struct User {
  std::string name;
  std::string tag;
};

bool locked = false;
int user_id = 0;
std::vector<User> user_list;

void handleRoot() {
  server.send_P(200, PSTR("text/html"), dist_index_html);
}

void handleNetworkGet() {
  char buffer[256];
  JsonDocument doc;
  doc["ssid"] = preferences.getString("ssid", "");
  doc["password"] = preferences.getString("password", "");
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void handleNetworkSave() {
  char buffer[256];
  JsonDocument doc;
  auto error = deserializeJson(doc, server.arg("plain"));
  if (error != DeserializationError::Ok) {
    doc.clear();
    doc["succeed"] = false;
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    return;
  }

  const std::string ssid = doc["ssid"];
  const std::string password = doc["password"];
  preferences.putString("ssid", ssid.c_str());
  preferences.putString("password", password.c_str());
  preferences.end();

  doc.clear();
  doc["succeed"] = true;
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void handleNetworkReset() {
  char buffer[256];
  JsonDocument doc;
  doc["succeed"] = false;
  serializeJson(doc, buffer);
  preferences.putString("ssid", "");
  preferences.putString("password", "");
  preferences.end();
  server.send(200, "application/json", buffer);
}

void handleLockState() {
  char buffer[256];
  JsonDocument doc;
  doc["locked"] = locked;
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void handleLockSet() {
  char buffer[256];
  JsonDocument doc;
  auto error = deserializeJson(doc, server.arg("plain"));
  if (error != DeserializationError::Ok) {
    doc.clear();
    doc["succeed"] = false;
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    return;
  }

  doc.clear();
  doc["succeed"] = true;
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void handleUserGet() {
  char buffer[512];
  JsonDocument doc;
  JsonArray items = doc["users"].to<JsonArray>();
  for (auto it = user_list.cbegin(); it != user_list.cend(); ++it) {
    auto item = items.createNestedObject();
    item["name"] = it->name;
    item["tag"] = it->tag;
  }
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void handleUserAdd() {
  char buffer[256];
  JsonDocument doc;
  auto error = deserializeJson(doc, server.arg("plain"));
  if (error != DeserializationError::Ok) {
    doc.clear();
    doc["succeed"] = false;
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    return;
  }

  const std::string name = doc["name"];
  const std::string tag = "tag_" + std::to_string(user_id++);
  user_list.emplace_back(User{name, tag});

  doc.clear();
  doc["succeed"] = true;
  doc["item"]["name"] = user_list.back().name;
  doc["item"]["tag"] = user_list.back().tag;
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void handleUserDelete() {
  char buffer[256];
  JsonDocument doc;
  auto error = deserializeJson(doc, server.arg("plain"));
  if (error != DeserializationError::Ok) {
    doc.clear();
    doc["succeed"] = false;
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    return;
  }

  // delete user by tag
  const std::string tag = doc["tag"];
  for (auto it = user_list.begin(); it != user_list.end(); ++it) {
    if (it->tag == tag) {
      user_list.erase(it);
      break;
    }
  }

  doc.clear();
  doc["succeed"] = true;
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void setup() {
  Serial.begin(115200);
  preferences.begin("my-app", false);

  char ssid[128] = {0};
  char password[128] = {0};
  bool has_ssid = preferences.getString("ssid", ssid, sizeof(ssid));
  bool has_password = preferences.getString("password", password, sizeof(password));
  if (!has_ssid || !has_password || !strlen(ssid)) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hotspot_ssid, hotspot_password);
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");


    // Wait for connection
    int countdown = 80;
    while (WiFi.status() != WL_CONNECTED && --countdown) {
      delay(500);
      Serial.print(".");
    }

    // revert to hotspot if having tried too many times
    if (!countdown) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(hotspot_ssid, hotspot_password);
    } else {
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      if (MDNS.begin("esp32")) {
        Serial.println("MDNS responder started");
      }
    }
  }

  server.on("/", handleRoot);
  server.on("/api/network/get", handleNetworkGet);
  server.on("/api/network/save", handleNetworkSave);
  server.on("/api/network/reset", handleNetworkReset);
  server.on("/api/control/lock/state", handleLockState);
  server.on("/api/control/lock/set", handleLockSet);
  server.on("/api/users/get", handleUserGet);
  server.on("/api/users/add", handleUserAdd);
  server.on("/api/users/delete", handleUserDelete);

  server.begin();
  Serial.println("HTTP server started");

  // make some dummy users
  user_list.emplace_back(User{"admin", "foo"});
  user_list.emplace_back(User{"guest", "bar"});
}

void loop() {
  server.handleClient();
  delay(2);  //allow the cpu to switch to other tasks
}
