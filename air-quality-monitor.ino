#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>

const int MQ2_analogPin = 25;
const int MQ2_digitalPin = 4;
const int PINO_LED_GREEN = 15;
const int PINO_LED_RED = 2;
const int PINO_BUZZER = 23;
const int CANAL_PWM = 0;

#define PINO_ANALOGIC 34
#define PINO_DIGITAL 32

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *ssid = "VIVOFIBRA-9821"; // Nome da rede Wi-Fi
const char *password = "nbKHs7TXPG"; // Senha da rede Wi-Fi
const char* mqtt_server = "test.mosquitto.org"; // Servidor MQTT

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long sensorStartTime;
unsigned long sensorEndTime;
unsigned long actuatorStartTime;
unsigned long actuatorEndTime;

void setup() {
  lcd.init();
  lcd.backlight();
 
  pinMode(PINO_LED_GREEN, OUTPUT);
  pinMode(PINO_LED_RED, OUTPUT);

  Serial.begin(9600);
  pinMode(PINO_DIGITAL, INPUT);

  ledcSetup(CANAL_PWM, 1000, 8);
  ledcAttachPin(PINO_BUZZER, CANAL_PWM);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle message arrived
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("seu_topico_subscrito");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int analogicValue = analogRead(PINO_ANALOGIC);
  int digitalValue = digitalRead(PINO_DIGITAL);

  Serial.print("Valor Analogico: ");
  Serial.print(analogicValue);
  Serial.print("\tValor Digital: ");
  Serial.println(digitalValue);

  if (digitalValue == HIGH) {
    sensorStartTime = millis(); // Tempo de detecção pelo sensor
    lcd.setCursor(0, 0);
    lcd.println("Qualidade do Ar:");
    lcd.setCursor(0, 1);
    lcd.println("RUIM");
    digitalWrite(PINO_LED_RED, HIGH);
    digitalWrite(PINO_LED_GREEN, LOW);
    ledcWrite(CANAL_PWM, 100);
    client.publish("qualidade/ar", "Alerta de Fumaca");
    delay(5000);
    sensorEndTime = millis(); // Tempo de registro pelo microcontrolador

    // Tempo de resposta do sensor
    unsigned long sensorResponseTime = sensorEndTime - sensorStartTime;

    // Tempo de resposta do atuador
    actuatorStartTime = millis(); // Tempo de ativação do atuador
    delay(1000); // Simulação de ativação do atuador
    actuatorEndTime = millis(); // Tempo de desativação do atuador
    unsigned long actuatorResponseTime = actuatorEndTime - actuatorStartTime;

    Serial.print("Tempo de Resposta do Sensor: ");
    Serial.print(sensorResponseTime);
    Serial.println(" ms");
    Serial.print("Tempo de Resposta do Atuador: ");
    Serial.print(actuatorResponseTime);
    Serial.println(" ms");
  } else {
    lcd.setCursor(0, 0);
    lcd.println("Qualidade do Ar:");
    lcd.setCursor(0, 1);
    lcd.println("BOA");
    digitalWrite(PINO_LED_RED, LOW);
    digitalWrite(PINO_LED_GREEN, HIGH);
    ledcWrite(CANAL_PWM, 0);
    delay(1000);
  }
}