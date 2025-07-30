#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

// WiFi bilgilerini buraya gir
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 webserver with OTA systems</title>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: #f4f4f9;
      margin: 0;
      padding: 0;
    }
    header {
      background-color: #1e88e5;
      color: white;
      padding: 20px;
      text-align: center;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
    }
    .container {
      padding: 20px;
      max-width: 900px;
      margin: auto;
    }
    .card {
      background: white;
      border-radius: 10px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    .card h2 {
      color: #1e88e5;
    }
    .footer {
      background-color: #ddd;
      padding: 10px;
      text-align: center;
      font-size: 14px;
      color: #555;
    }
    .project {
      margin-bottom: 15px;
    }
    .project-title {
      font-size: 18px;
      color: #333;
      font-weight: bold;
    }
    .project-description {
      font-size: 14px;
      color: #555;
    }
    a {
      color: #1e88e5;
      text-decoration: none;
    }
    a:hover {
      text-decoration: underline;
    }
    input[type=file] {
      padding: 5px;
    }
    input[type=submit] {
      background-color: #1e88e5;
      border: none;
      color: white;
      padding: 10px 15px;
      font-size: 16px;
      border-radius: 5px;
      cursor: pointer;
      margin-top: 10px;
    }
    input[type=submit]:hover {
      background-color: #1565c0;
    }
  </style>
</head>
<body>
  <header>
    <h1>ESP32 Smart Home Controller</h1>
    <p>by Emir Kaan Demirdaş</p>
  </header>
  <div class="container">
    <div class="card">
      <h2>My Projects</h2>

      <div class="project">
        <div class="project-title">🚪 Smart Door Control System</div>
        <div class="project-description">An ESP32-based system providing automated door operation using an LCD display, LED indicators, button controls, and a motor driver. Features serial monitoring for live system status updates.</div>
      </div>

      <div class="project">
        <div class="project-title">🌱 GreenHouse IoT System</div>
        <div class="project-description">A comprehensive environmental monitoring system designed for vertical farming. Based on the ESP32, it provides live data tracking and automation via a web-based interface.</div>
      </div>

      <div class="project">
        <div class="project-title">💡 IoT Lighting System</div>
        <div class="project-description">WiFi-controlled lighting system using ESP32 and relays. Users can control lighting remotely via a web interface with JSON API support.</div>
      </div>

      <div class="project">
        <div class="project-title">📊 PIC-based Weather Station</div>
        <div class="project-description">Built with the PIC18F452 microcontroller, this project enables multi-sensor monitoring, automatic irrigation, and climate control, targeting smart agriculture applications.</div>
      </div>
    </div>

    <div class="card">
      <h2>🔁 OTA Update Panel</h2>
      <p>Use the form below to upload a new firmware (.bin) file to your ESP32 device for over-the-air updates.</p>
      <form method="POST" action="/update" enctype="multipart/form-data">
        <input type="file" name="update" accept=".bin" required />
        <input type="submit" value="Upload Firmware" />
      </form>
    </div>
  </div>

  <div class="footer">
    © 2025 Emir Kaan Demirdaş - ESP32 Developer Platform
  </div>
</body>
</html>
)rawliteral";

// Handler for root page
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// OTA firmware upload handler
void handleUpdate() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Start updating: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write chunk to flash
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { // true to set boot partition to new
      Serial.printf("Update Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Serial.println("Update aborted");
  }
  yield();
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");

  delay(1000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, [](){
    server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
  }, handleUpdate);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
