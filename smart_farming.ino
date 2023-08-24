#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>

#define DHTPIN D2  // Temp + Humidity Sensor
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN A0
#define RELAY_PIN D1
#define SWITCH_PIN D3


DHT dht(DHTPIN, DHTTYPE);

// Set these to run example.
#define FIREBASE_HOST "https://smart-irrigation-c2635-default-rtdb.asia-southeast1.firebasedatabase.app"  //Firebase database URL
#define FIREBASE_AUTH "AIzaSyD7qUG_HiyhgOESewb9G153iiF8SfFcHKE"                                           // API Key
#define WIFI_SSID "Your"                                                                                  //WiFi Name
#define WIFI_PASSWORD "123456789"                                                                         //WiFi Password

FirebaseData firebaseData;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, HIGH);  // Assume the Relay is Active LOW

  dht.begin();  // Initializing temperature sensor

  // Connecting to Wi-Fi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();



  // Connecting to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Initialize motor state to false in Firebase
  Firebase.setBool(firebaseData, "/motorState", false);
}

void loop() {
  delay(2000);  // delay 2 Seconds


  // Manual Switch Control
  static bool prevSwitchState = HIGH;
  bool switchState = digitalRead(SWITCH_PIN);
  if (switchState != prevSwitchState) {
    prevSwitchState = switchState;
    bool relayState = (switchState == LOW);
    digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
    Firebase.setBool(firebaseData, "/motorState", relayState);
  }


  float humidity = dht.readHumidity();        // Himidity
  float temperature = dht.readTemperature();  // Temperature

  int raw_soil_moisture = analogRead(SOIL_MOISTURE_PIN);  //Soil Moisture
  int soil_moisture = map(raw_soil_moisture, 1023, 10, 0, 100);


  // Error Handling
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  //Pushing Sensors Data to Firebase RTDB
  Firebase.setFloat(firebaseData, "/temperature", temperature);
  Firebase.setFloat(firebaseData, "/humidity", humidity);
  Firebase.setInt(firebaseData, "/soil_moisture", soil_moisture);

  // Read motor state from Firebase
  if (Firebase.getBool(firebaseData, "/motorState")) {
    if (firebaseData.boolData() == true) {
      digitalWrite(RELAY_PIN, LOW);  // Assuming relay is Active LOW
    } else {
      digitalWrite(RELAY_PIN, HIGH);
    }
  }

  // Serial Print Statements
  Serial.print("Temperature : ");
  Serial.println(temperature);
  Serial.print("Humidity : ");
  Serial.println(humidity);
  Serial.print("Soil Moisture : ");
  Serial.println(soil_moisture);
  Serial.println("<------------->");
}
