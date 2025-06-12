#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
const char* ssid = "AE's iPhone";
const char* password = "1234567890";

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600, 60000); // For Uzbekistan

const int moisturePin = A0;
const int pumpPin = D1;

bool pumpOn = false;
int irrigationHour = 6;    // Hour for watering
int irrigationMinute = 0;  // Minute for watering
void setup() {
  Serial.begin(115200);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/toggle", handleToggle);

  server.begin();
  Serial.println("Server is working");
}
void loop() {
  server.handleClient();
  timeClient.update();

  int currentH = timeClient.getHours();
  int currentM = timeClient.getMinutes();

  if (currentH == irrigationHour && currentM == irrigationMinute && !pumpOn) {
    digitalWrite(pumpPin, HIGH);
    pumpOn = true;
  } else if (currentH != irrigationHour || currentM != irrigationMinute) {
    digitalWrite(pumpPin, LOW);
    pumpOn = false;
  }
}
void handleData() {
  int raw = analogRead(moisturePin);
  int moisture = map(raw, 1023, 0, 0, 100);  // 0-100% sensor
  String currentTime = timeClient.getFormattedTime();
  char buf[8];
  sprintf(buf, "%02d:%02d", irrigationHour, irrigationMinute);

  String json = "{";
  json += "\"moisture\":" + String(moisture) + ",";
  json += "\"time\":\"" + currentTime + "\",";
  json += "\"irrigation\":\"" + String(buf) + "\",";
  json += "\"pump\":" + String(pumpOn ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void handleToggle() {
  pumpOn = !pumpOn;
  digitalWrite(pumpPin, pumpOn ? HIGH : LOW);
  server.send(200, "text/plain", "OK");
}
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Smart Garden</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background: linear-gradient(to right, #f1f8e9, #b2dfdb);
      text-align: center;
      padding: 20px;
    }
    h1 { color: #2e7d32; }
    .btn {
      background: linear-gradient(to right, #43cea2, #185a9d);
      color: white;
      padding: 15px 25px;
      font-size: 18px;
      border-radius: 30px;
      border: none;
      margin-top: 20px;
      cursor: pointer;
    }
    canvas { margin: 20px auto; }
    .info { font-size: 18px; margin: 10px 0; }
  </style>
</head>
<body>
  <h1> Smart Irrigation Panel</h1>
  <canvas id="gauge" width="300" height="150"></canvas>
  <div class="info">
    <p><strong> Humidity:</strong> <span id="moisture">--</span>%</p>
    <p><strong> Time:</strong> <span id="time">--:--:--</span></p>
    <p><strong> Watering:</strong> <span id="irrigation">--:--</span></p>
    <p><strong> Pump:</strong> <span id="pump">---</span></p>
  </div>
  <button class="btn" onclick="togglePump()">Activate/deactivate pump</button>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script>
    const ctx = document.getElementById('gauge').getContext('2d');
    const chart = new Chart(ctx, {
      type: 'doughnut',
      data: {
        datasets: [{
          data: [0, 100],
          backgroundColor: ['#00c853', '#eeeeee'],
          borderWidth: 0
        }]
      },
      options: {
        rotation: 270,
        circumference: 180,
        cutout: '75%',
        plugins: {
          tooltip: { enabled: false },
          legend: { display: false }
        }
      }
    });

    async function fetchData() {
      const res = await fetch('/data');
      const data = await res.json();
      document.getElementById("moisture").textContent = data.moisture;
      document.getElementById("time").textContent = data.time;
      document.getElementById("irrigation").textContent = data.irrigation;
      document.getElementById("pump").textContent = data.pump ? "Burn" : "Ouchy";

      chart.data.datasets[0].data = [data.moisture, 100 - data.moisture];
      chart.update();
    }

    function togglePump() {
      fetch('/toggle').then(fetchData);
    }

    fetchData();
    setInterval(fetchData, 5000);
  </script>
</body>
</html>
  )rawliteral";

  server.send(200, "text/html", html);
}
