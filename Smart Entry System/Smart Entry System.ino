//These are required for Blynk IoT cloud to link your code with the device you created online
#define BLYNK_TEMPLATE_ID "TMPL6BJ-sUWUH"
#define BLYNK_TEMPLATE_NAME "IOT RFID Door Lock"
#define BLYNK_AUTH_TOKEN "t0Y7OVJuYyicGCUR9CsflUyK1mIwtV0R"

#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <map>
#include "esp_camera.h"

#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"



// const char* ssid = "A T patel";
// const char* pass = "Tirth9192";
// const char* ssid = "Wi5G";
// const char* pass = "Qwerty@5";
// const char* ssid = "Telstra19FF";
// const char* pass = "9982236059";
const char* ssid = "Sandeep";
const char* pass = "sandeep098";


const char* firebaseHostRDB = "https://iot-rfid-door-lock-default-rtdb.asia-southeast1.firebasedatabase.app/";
// const char* firebaseHostStorage = "firebasestorage.googleapis.com";
// const char* firebaseBucket = "iot-rfid-door-lock.firebasestorage.app";

// --- Blynk Virtual Pin Assignments ---

// RFID Inputs & Info Display
#define VPIN_UID_DISPLAY        V0  // Displays scanned UID for addition
#define VPIN_NAME_INPUT         V1  // Input field for name
#define VPIN_STATUS_SWITCH      V2  // Switch to set user status (active/disabled)
#define VPIN_ADD_BUTTON         V3  // Button to add/update user
#define VPIN_MANUAL_RELAY       V4  // Button to open/close door manually
#define VPIN_UID_DISPLAY_DELETE V5  // Shows UID for deletion
#define VPIN_DELETE_BUTTON      V6  // Button to delete user

// LCD Display
#define VPIN_LCD               V7  // LCD widget (optional, defined with WidgetLCD)

// Logs Terminal
#define VPIN_TERMINAL_AUTHORIZED   V8  // Terminal for authorized access logs
#define VPIN_TERMINAL_UNAUTHORIZED V9  // Terminal for unauthorized access logs

// Refresh Button
#define VPIN_REFRESH_LOGS      V10  // Button to refresh logs display
//----------------------------------------

//RELAY_PIN is the GPIO used to control the relay (or LED).
#define RELAY_PIN 2        // GPIO2 (connect relay/LED here)

// RFID pins (custom SPI)
#define SS_PIN 5
#define RST_PIN 4
#define SCK_PIN 7
#define MOSI_PIN 9
#define MISO_PIN 8

// Maximum limit to show logged data
#define MAX_LOG_DATA 10


/*
Creates an RFID reader object called mfrc522.
BlynkTimer lets you run functions at set intervals (e.g., check RFID every 500ms).
*/
MFRC522 mfrc522(SS_PIN, RST_PIN);
BlynkTimer timer;

WidgetLCD lcd(VPIN_LCD);
WidgetTerminal terminal1(VPIN_TERMINAL_AUTHORIZED), terminal2(VPIN_TERMINAL_UNAUTHORIZED); 


String scanned_uid = "";
String last_scanned_uid = "";
int messageShownTime = 0;
int unauthAttemptCount = 0;

struct RFIDUser {
  String uid;
  String name;
  int status;
  String last_access = "never" ;
} currentUser;

struct UserInfo {
  String name;
  String status;
};
std::map<String, UserInfo> rfidUsers;  // UID → UserInfo


class CameraUploader {
  private:
    const char* firebaseHost = "firebasestorage.googleapis.com";
    const char* firebaseBucket = "iot-rfid-door-lock.firebasestorage.app";
    camera_fb_t* fb;

  public:
    CameraUploader() : fb(nullptr) {}

    camera_config_t getCameraConfig() {
      camera_config_t config;
      config.ledc_channel = LEDC_CHANNEL_0;
      config.ledc_timer = LEDC_TIMER_0;
      config.pin_d0 = Y2_GPIO_NUM;
      config.pin_d1 = Y3_GPIO_NUM;
      config.pin_d2 = Y4_GPIO_NUM;
      config.pin_d3 = Y5_GPIO_NUM;
      config.pin_d4 = Y6_GPIO_NUM;
      config.pin_d5 = Y7_GPIO_NUM;
      config.pin_d6 = Y8_GPIO_NUM;
      config.pin_d7 = Y9_GPIO_NUM;
      config.pin_xclk = XCLK_GPIO_NUM;
      config.pin_pclk = PCLK_GPIO_NUM;
      config.pin_vsync = VSYNC_GPIO_NUM;
      config.pin_href = HREF_GPIO_NUM;
      config.pin_sccb_sda = SIOD_GPIO_NUM;
      config.pin_sccb_scl = SIOC_GPIO_NUM;
      config.pin_pwdn = PWDN_GPIO_NUM;
      config.pin_reset = RESET_GPIO_NUM;
      config.xclk_freq_hz = 20000000;
      config.pixel_format = PIXFORMAT_JPEG;
      // config.frame_size = FRAMESIZE_SVGA;
      config.frame_size = FRAMESIZE_VGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
      config.fb_location = CAMERA_FB_IN_PSRAM;
      config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

      if (config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
          // config.jpeg_quality = 8;
          // config.fb_count = 2;
          config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
          // config.frame_size = FRAMESIZE_SVGA;
          config.fb_location = CAMERA_FB_IN_DRAM;
        }
      } else {
        config.frame_size = FRAMESIZE_240X240;
      }

      return config;
    }

    void adjustSensorSettings() {
      sensor_t* s = esp_camera_sensor_get();
      if (!s) {
        Serial.println("Failed to get camera sensor pointer");
        return;
      }
      s->set_brightness(s, 1);
      s->set_saturation(s, -1);
      s->set_contrast(s, -2);
    }

    bool initCamera() {
      camera_config_t config = getCameraConfig();
      if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera init failed");
        return false;
      }
      adjustSensorSettings();
      return true;
    }

    bool capturePhoto() {
      this->fb = esp_camera_fb_get();
      if (!this->fb) {
        Serial.println("Camera capture failed");
        return false;
      }
      Serial.println("Photo captured.");
      Serial.print("Image size (bytes): ");
      Serial.println(fb->len);
      Serial.print("Image size (KB): ");
      Serial.println(fb->len / 1024.0, 2);
      return true;
    }

    String uploadToFirebase() {
      WiFiClientSecure client;
      client.setInsecure();

      HTTPClient https;
      https.setTimeout(15000);  // <-- Increase timeout

      String fileName = "photo_" + String(time(nullptr)) + ".jpg";
      String url = "https://" + String(this->firebaseHost) + "/v0/b/" + String(this->firebaseBucket) + "/o?uploadType=media&name=" + fileName;
      String publicURL = "";
      Serial.println(url);

      if (!https.begin(client, url)) {
        Serial.println("[ERROR] Failed to begin HTTPS connection");
        return "";
      }
Serial.printf("Free heap before upload: %d bytes\n", ESP.getFreeHeap());

      https.addHeader("Content-Type", "image/jpeg");
      int retryCount = 0;
      bool success = false;
      while (retryCount < 3 && !success) {
          int httpResponseCode = https.POST(this->fb->buf, this->fb->len);

          if (httpResponseCode > 0) {
            Serial.printf("Image Upload success [%d]\n", httpResponseCode);
            String response = https.getString();
            // Serial.println("Firebase Response:");
            // Serial.println(response);
            Serial.println("Public URL:");
            Serial.println("https://" + String(this->firebaseHost) + "/v0/b/" + String(this->firebaseBucket) + "/o/" + fileName + "?alt=media");
            publicURL = "https://" + String(this->firebaseHost) + "/v0/b/" + String(this->firebaseBucket) + "/o/" + fileName + "?alt=media";
            success = true;
          } else {
            Serial.printf("Upload failed: %s\n", https.errorToString(httpResponseCode).c_str());
            retryCount++;
          }
      }
      https.end();
      esp_camera_fb_return(fb);
      fb = nullptr;
      return publicURL;
    }
};
CameraUploader uploader;

void clearDisplay()
{
  lcd.clear();
  // lcd.print(0, 0, "Ready to scan");
}
void show_message(String msg)
{
  lcd.clear();
  lcd.print(0,0,msg);
  messageShownTime = millis();
}
void clear_fields(){
  scanned_uid = "";
  currentUser.uid = "";
  currentUser.name = "";
  currentUser.status = 1;
  currentUser.last_access = "never";
  //  Clear fields on app screen after submission
  Blynk.virtualWrite(VPIN_UID_DISPLAY, " ");  // Clear UID display
  Blynk.virtualWrite(VPIN_NAME_INPUT, " ");   // Clear name input
    // Blynk.setProperty(V1,"value",""); Not working
  Blynk.virtualWrite(VPIN_STATUS_SWITCH, 0);    // Reset status switch to 0 (inactive)
  Blynk.virtualWrite(VPIN_UID_DISPLAY_DELETE, " ");
  unauthAttemptCount = 0;
  last_scanned_uid = "";
  
  Serial.println("Form submitted and cleared.");
}

String getTimestamp() {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  char buf[26];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", 
          t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec);
  return String(buf);
}

void updateLastAccess(String uid, String timestamp) {
  String url = String(firebaseHostRDB) + "rfid_users/" + uid + "/last_access.json";

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Send the PUT request
  int httpResponseCode = http.PUT("\"" + timestamp + "\""); // Firebase requires quotes for strings

  if (httpResponseCode > 0) {
    // Serial.println("HTTP Response code: " + String(httpResponseCode));
    if (httpResponseCode == 200) {
      Serial.println("last_access successfully updated for UID: " + uid + " : " + timestamp);
    } else {
      Serial.println("Warning: Non-200 response. Check Firebase rules or formatting.");
    }
  } else {
    Serial.println("HTTP request failed(updateLastAccess). Error: " + http.errorToString(httpResponseCode));
  }

  http.end();
}


// Blynk Button: Turn relay on/off manually
BLYNK_WRITE(VPIN_MANUAL_RELAY) {
  int state = param.asInt();
  // Serial.println(state);
  digitalWrite(RELAY_PIN, state);
  if(state == 1)
    show_message("Door Opened"); // (col, row, text)
  else
    show_message("Door Closed"); // (col, row, text)
}


// Make this function async
void fetchRFIDUsersFromFirebase() {
  HTTPClient http;
  String url = String(firebaseHostRDB) + "rfid_users.json";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();

  if (httpCode == 200) 
  {
    String payload = http.getString();
    Serial.println("Received user list from Firebase:");

    DynamicJsonDocument json(32768);  // 32KB Increase if needed
    DeserializationError error = deserializeJson(json, payload);
    if (error) {
      Serial.println("JSON parse failed!");
      return;
    }

    // empting the list first
    rfidUsers.clear();

    for (JsonPair kv : json.as<JsonObject>()) 
    {
      String uid = kv.key().c_str();
      JsonObject userData = kv.value().as<JsonObject>();
      UserInfo user;

      user.name = userData["name"].as<String>();
      user.status = userData["status"].as<String>();

      rfidUsers[uid] = user;

      Serial.println( uid + " | " + user.name + " | " + user.status);
    }
  } else {
    Serial.println("HTTP GET failed with code (fetchRFID): " + String(httpCode));
  }

  http.end();
}

// Check RFID card every 500 ms
void checkRFID() {

  //This checks if a new RFID tag is present and readable.
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    if (mfrc522.uid.uidByte[i] < 0x10) uidString += "0";
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }

  uidString.toUpperCase(); // optional, for cleaner format like "43007BB7"
  scanned_uid = uidString;

  if(last_scanned_uid == "")
    last_scanned_uid = scanned_uid;

  currentUser.uid = uidString;
  String timestamp = getTimestamp();
  currentUser.last_access = timestamp;
  Serial.println("Scanned UID: " + uidString);

  if (Blynk.connected()) {
    Blynk.virtualWrite(VPIN_UID_DISPLAY, uidString);
    Blynk.virtualWrite(VPIN_UID_DISPLAY_DELETE, uidString);
  }

  auto it = rfidUsers.find(uidString);
  if (it != rfidUsers.end()) 
  {
    UserInfo user = it->second;
    currentUser.name = user.name;
    currentUser.status = user.status == "active" ? 1 : 0;

    // Collecting this info for updating details
    Blynk.virtualWrite(VPIN_UID_DISPLAY, currentUser.uid);  // Clear UID display
    Blynk.virtualWrite(VPIN_NAME_INPUT, currentUser.name);   // Clear name input
    Blynk.virtualWrite(VPIN_STATUS_SWITCH, currentUser.status);

    if (user.status == "active") 
    {
      //--------------------------------------------------------
      Serial.println("Access granted for: " + user.name);
      show_message("Door Opened");
      digitalWrite(RELAY_PIN, HIGH);
      delay(3000);
      digitalWrite(RELAY_PIN, LOW);
      show_message("Door Closed");

      updateLastAccess(currentUser.uid, currentUser.last_access);
      logAuthorizedAccess(currentUser.uid, currentUser.name, currentUser.last_access);
      // delay(3000);
      // clear_fields();
    } 
    else 
    {
      if(scanned_uid == last_scanned_uid){
        unauthAttemptCount++;
      }
      else{
        clear_fields();
        unauthAttemptCount++;
      }
      
      Serial.println(user.name + " Card is inactive.");
      Serial.println("Unauthorized attempt #" + String(unauthAttemptCount));
      show_message("Attempt #" + String(unauthAttemptCount));
      updateLastAccess(currentUser.uid, currentUser.last_access);

      if (unauthAttemptCount >= 3) {
        String photoUrl = "";
        if (uploader.capturePhoto()){
          photoUrl = uploader.uploadToFirebase();
        }
        logUnauthorizedAccess(currentUser.uid, currentUser.name != "" ? currentUser.name : "Unknown", currentUser.last_access, photoUrl);
        clear_fields();
      }
      // Serial.println(currentUser.uid);
      // Serial.println(currentUser.name);
      // Serial.println(currentUser.status);
      // Serial.println(currentUser.last_access);
      // lcd.print(0, 0, "Card is inactive.");
    }
  } 
  else 
  {
    if(scanned_uid == last_scanned_uid){
      unauthAttemptCount++;
    }
    else{
      clear_fields();
      unauthAttemptCount++;
    }
    Serial.println("Unregistered User");
    Serial.println("Unauthorized attempt #" + String(unauthAttemptCount));
    show_message("Attempt #" + String(unauthAttemptCount));

    if (unauthAttemptCount >= 3) {
      String photoUrl = "";
      if (uploader.capturePhoto()){
        photoUrl = uploader.uploadToFirebase();
      }
      logUnauthorizedAccess(currentUser.uid, currentUser.name != "" ? currentUser.name : "Unknown", currentUser.last_access, photoUrl);
      clear_fields();
    }
    // logUnauthorizedAccess(currentUser.uid, currentUser.name, currentUser.last_access);
    // lcd.print(0, 0, "Unregistered User");
  }
// Stop reading this card
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();  // Also stop encryption if authentication was used
}

BLYNK_WRITE(VPIN_UID_DISPLAY) { currentUser.uid = param.asString(); }
BLYNK_WRITE(VPIN_NAME_INPUT) { currentUser.name = param.asString(); }
BLYNK_WRITE(VPIN_STATUS_SWITCH) { currentUser.status = param.asInt(); }

// Add user
BLYNK_WRITE(VPIN_ADD_BUTTON) {
  if(param.asInt() == 0) return; // Fucntion is called twice for On and Off state.
  String uid = currentUser.uid;         // add_uid Not using currentUser because could be null if user is not registered
  String name = currentUser.name;      // add_name
  int status = currentUser.status;          // add_status
  String last_access = currentUser.last_access;

  if (uid == "" || name == "") 
  {
    Serial.println("************** Fields Empty! Submission canceled.*****************");
    show_message("ID or Name Fields cannot be empty"); // (col, row, text)
    return;
  }

  String path = "rfid_users/" + uid + ".json";
  String url = firebaseHostRDB + path; // No auth in test mode otherwise we will need secret key
  Serial.println("Firebase URL: " + url);  // Debug print only, doesn't send

  // Prepare JSON data
  DynamicJsonDocument json(256);
  json["name"] = name;
  json["status"] = status == 1 ? "active" : "disabled";
  json["last_access"] = last_access;

  String body;
  serializeJson(json, body);
  Serial.println("Prepared JSON: " + body);  // Just previewing, not sending yet

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(body);  // Perform the PUT request

  if (httpCode == 200 || httpCode == 201) 
  {
    // if (last_access == "never")
    // {
      Serial.println("User added successfully to Firebase.");
      show_message("User added");
    // }
    // else
    // {
    //   Serial.println("User update successfully to Firebase.");
    //   show_message("Details Updated");
    // }
    
  }
  else 
  {
    Serial.println("Failed to add. HTTP code(Add): " + String(httpCode));
    show_message("Registration Failed");
  }
  http.end();
  clear_fields();
}

// Delete User
BLYNK_WRITE(VPIN_DELETE_BUTTON) 
{
  if (param.asInt() == 0) return; // Fucntion is called twice for On and Off state.

  if (scanned_uid == "") 
  {
    Serial.println("No UID provided to delete.");
    show_message("Need to scan RFID"); // (col, row, text)
    return;
  }

  if (rfidUsers.find(scanned_uid) == rfidUsers.end()) {
    show_message("User not found");
    return;
  }


  String url = String(firebaseHostRDB) + "rfid_users/" + scanned_uid + ".json";

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.sendRequest("DELETE");

  if (httpCode == 200) 
  {
    Serial.println("UID " + scanned_uid + " deleted from Firebase.");
    show_message("User Deleted");
  } 
  else 
  {
    Serial.println("Failed to delete. HTTP code(remove): " + String(httpCode));
    show_message("Deletion Failed");
  }
  http.end();
  clear_fields();
}

void logToTerminal(String logEntry, String logtype) {
  if(logtype == "authorized_logs")
  {
    terminal1.println(logEntry);
    // terminal1.flush(); // Force send to app
  }
  else
  {
    terminal2.println(logEntry);
    // terminal2.flush(); // Force send to app
  }
  
}

void logAuthorizedAccess(String uid, String name, String timestamp) {
  // Using current time as logId so that sorting of log is possible
  time_t now = time(nullptr);
  String logId = "log_" + String(now);

  // Push to Firebase
  String url = String(firebaseHostRDB) + "/authorized_logs/" + logId + ".json";
  DynamicJsonDocument json(256);
  json["uid"] = uid;
  json["name"] = name;
  json["timestamp"] = timestamp;

  String payload;
  serializeJson(json, payload);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.PUT(payload);

  if (httpResponseCode > 0) {
    Serial.println("Authorized access log response: " + String(httpResponseCode));
    if (httpResponseCode == 200) {
      Serial.println("Logged authorized access: " + uid + " - " + name + " - " + timestamp);
    } else {
      Serial.println("Unexpected response when logging authorized access.");
    }
  } else {
    Serial.println("Failed to log authorized access(log). Error: " + http.errorToString(httpResponseCode));
  }

  http.end();
}

void logUnauthorizedAccess(String uid, String name, String timestamp, String photoUrl){
  // Using current time as logId so that sorting of log is possible
  time_t now = time(nullptr);
  String logId = "log_" + String(now);

  String url = String(firebaseHostRDB) + "/unauthorized_logs/" + logId + ".json";
  DynamicJsonDocument json(256);
  json["uid"] = uid;
  json["name"] = name;
  json["timestamp"] = timestamp;
  json["photoUrl"] = photoUrl;

  String payload;
  serializeJson(json, payload);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.PUT(payload);
  if (httpResponseCode > 0) {
    Serial.println("Unauthorized access log response: " + String(httpResponseCode));
    if (httpResponseCode == 200) {
      Serial.println("Logged unauthorized access: " + uid + " - " + name + " - " + timestamp);
    } else {
      Serial.println("Unexpected response when logging unauthorized access.");
    }
  } else {
    Serial.println("Failed to log unauthorized access. Error(log): " + http.errorToString(httpResponseCode));
  }

  http.end();
  String emailMessage = "Unauthorized access attempt detected!\n";
  emailMessage += "UID: " + uid + "\n";
  emailMessage += "Name: " + name + "\n";
  emailMessage += "Time: " + timestamp + "\n";
  emailMessage += "Photo: " + photoUrl;
  Blynk.logEvent("unauthorized_access", emailMessage);
}

BLYNK_WRITE(VPIN_REFRESH_LOGS) 
{
  if (param.asInt() == 0) return; // Fucntion is called twice for On and Off state.
  terminal1.clear();
  terminal2.clear();
  fetchAndDisplayLogs("authorized_logs");
  fetchAndDisplayLogs("unauthorized_logs");
}

void fetchAndDisplayLogs(String logType) {
  HTTPClient http;
  String url = firebaseHostRDB + logType + ".json";
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode != 200) {
    Serial.println("Failed to fetch logs from " + logType);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument json(8192); // or increase if logs are large
  deserializeJson(json, payload);

  std::vector<std::pair<String, JsonObject>> logs;

  for (JsonPair kv : json.as<JsonObject>()) {
    logs.push_back({kv.key().c_str(), kv.value().as<JsonObject>()});
  }

  // Sort by log ID (which includes millis timestamp)
  std::sort(logs.begin(), logs.end(), [](auto &a, auto &b) {
    return a.first > b.first; // descending
  });

  // Show top 10 logs
  int count = 0;

  for (auto &log : logs) {
    if (count++ >= MAX_LOG_DATA) break;
    String line;
    line = "[" + log.second["timestamp"].as<String>() +"] "+log.second["name"].as<String>() + " (" + log.second["uid"].as<String>() + ")"; 
    logToTerminal(line, logType);
  }
  terminal1.flush();
  terminal2.flush();
}


void connectWifi()
{
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void checkWiFiReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    connectWifi();
  }
}
bool initializeBlynk()
{
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if (Blynk.connected()) {
    Serial.println("Blynk is connected");
    return true;
  } else {
    Serial.println("Blynk is NOT connected");
    return false;
  }
}
void setup() 
{
  Serial.setDebugOutput(true);
  // Synchronized timezonw
  configTime(3600 * 8, 0, "pool.ntp.org", "time.nist.gov");  // Replace 8 with your timezone offset
  //  Initialize serial communication between your ESP32 (or other microcontroller) and your computer or serial monitor.
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  connectWifi();

  // To initialize the SPI (Serial Peripheral Interface) bus with custom-defined pins for SPI communication
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  // Initializing rc522
  mfrc522.PCD_Init();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Starts Blynk connection.
  if (!initializeBlynk()) return;

  // Initialize camera
  if (!uploader.initCamera()) return;

  fetchRFIDUsersFromFirebase();  // Load once at boot
  timer.setInterval(30000L, fetchRFIDUsersFromFirebase);  // Refresh every 60000-> 60 sec
  timer.setInterval(1000L, checkRFID);  // run check every 0.5 sec
  show_message("System Connected...");
  // timer.setInterval(2000L, clearDisplay);
}

void loop() 
{
  Blynk.run();
  timer.run();

  if (messageShownTime && millis() - messageShownTime > 2000) {
    clearDisplay();
   messageShownTime = 0;
  }
  checkWiFiReconnect();
}

