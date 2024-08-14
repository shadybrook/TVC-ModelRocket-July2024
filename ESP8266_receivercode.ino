#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RF24.h>

#define CE_PIN 5
#define CSN_PIN 4

RF24 radio(CE_PIN, CSN_PIN);  // CE, CSN pins

float angles[2] = {0.0, 0.0}; // Array to hold received angle X and Y

// Replace with your network credentials
const char* ssid = "Kiran_4G";
const char* password = "9820700378";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Initialize RF24
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setPayloadSize(sizeof(angles));
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL);  // Set the address for reading
  radio.startListening();  // Set the radio to RX mode

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup web server
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE HTML><html><head><meta charset=\"utf-8\"><title>Telemetry Data</title></head><body>";
    html += "<h1>Telemetry Data</h1>";
    html += "<p>Angle X: <span id=\"angleX\">0</span>°</p>";
    html += "<p>Angle Y: <span id=\"angleY\">0</span>°</p>";
    html += "<script>setInterval(function() { fetch('/data').then(response => response.json()).then(data => { document.getElementById('angleX').textContent = data.angleX; document.getElementById('angleY').textContent = data.angleY; }); }, 1000);</script></body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/data", HTTP_GET, []() {
    String json = "{";
    json += "\"angleX\":" + String(angles[0]) + ",";
    json += "\"angleY\":" + String(angles[1]);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  if (radio.available()) {
    radio.read(&angles, sizeof(angles));
    Serial.print("Angle X: ");
    Serial.print(angles[0]);
    Serial.print(", Angle Y: ");
    Serial.println(angles[1]);
  }

  server.handleClient();
}
