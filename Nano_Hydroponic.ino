#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <Adafruit_Sensor.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <SimpleTimer.h>

SimpleTimer timer;

float calibration_value = 7.0;  // Adjusted calibration value for tap water
float calibration_slope = -1.001;   // Slope adjustment factor for pH calculation
int buffer_arr[10], temp;
float ph_act;

char ssid[] = "Redmi";
char pass[] = "12345678";
char broker[] = "mqtt-dashboard.com";
const char topic1[] = "WaterLevel";
const char topic2[] = "DHT";
const char topic3[] = "Ph";

WiFiClient wifiClient;
MqttClient client(wifiClient);

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int waterSensorPin = A0;

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  dht.begin();

  // Connecting to broker
  while (!client.connect(broker, 1883)) {
    Serial.print("MQTT CONNECTION FAILED");
    Serial.println(client.connectError());
    delay(1000);
  }
}

void loop() {
  client.poll();
  int waterLevel = analogRead(waterSensorPin);
  float temperature = dht.readTemperature();
  if (client.connected()) {
    if (client.beginMessage(topic1)) {
      Serial.println("Published Water Level: " + String(waterLevel));
      client.println(waterLevel);
      client.endMessage();
    }

    if (client.beginMessage(topic2)) {
      Serial.println("Published Temperature: " + String(temperature));
      client.println(temperature);
      client.endMessage();
    }

    if (client.beginMessage(topic3)) {
      timer.run();

      // Collect and check analog readings from pH sensor
      for (int i = 0; i < 10; i++) {
        buffer_arr[i] = analogRead(A1);
        Serial.print("Raw pH Reading [");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(buffer_arr[i]);
        delay(50);
      }

      // If all values are zero, sensor might be disconnected
      bool allZero = true;
      for (int i = 0; i < 10; i++) {
        if (buffer_arr[i] != 0) {
          allZero = false;
          break;
        }
      }
      if (allZero) {
        Serial.println("pH Sensor may be disconnected or not functioning!");
        return;
      }

      // Sort and average middle values for stability
      for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
          if (buffer_arr[i] > buffer_arr[j]) {
            temp = buffer_arr[i];
            buffer_arr[i] = buffer_arr[j];
            buffer_arr[j] = temp;
          }
        }
      }

      unsigned long int avgval = 0;
      for (int i = 2; i < 8; i++) {
        avgval += buffer_arr[i];
      }

      // Calculate voltage and pH
      float volt = (float)avgval * 5.0 / 1023 / 6;
      Serial.print("Calculated Voltage: ");
      Serial.println(volt);

     ph_act = calibration_value - (calibration_slope * volt);


      Serial.print("Published pH Value: ");
      Serial.println(ph_act);

      client.print(ph_act);
      client.endMessage();
    }
    delay(1000);
  }

  if (isnan(temperature) || isnan(waterLevel) || isnan(ph_act)) {
    Serial.println(F("Failed to read data from DHT or pH Sensor!"));
    return;
  }

  Serial.print(F("Temperature: "));
  Serial.print(temperature);
  Serial.print(F("°C, Water Level: "));
  Serial.println(waterLevel);
  delay(1000);
}
