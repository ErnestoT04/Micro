#include <WiFi.h>
#include <PubSubClient.h>


const char* ssid = ".a";
const char* password = "ernesto12345678";
const char* mqtt_server = "broker.emqx.io";
int port = 1883;

WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);

// Pines del puente H
const int in1 = 4;  // Subir/bajar motor
const int in2 = 0;
const int in3 = 2;  // Abrir/cerrar puerta
const int in4 = 15;

//Pine display
const int dataPin = 12;   // DS (pin 14 del 74HC595)
const int clockPin = 14;  // SH_CP (pin 11 del 74HC595)
const int latchPin = 27;   // ST_CP (pin 12 del 74HC595)

byte numeros[] = {
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111  // 7
};




String resultS = "";

void wifiInit() {
  Serial.print("Conectándose a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado");
  Serial.println("IP: " + WiFi.localIP().toString());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");

  resultS = "";
  for (int i = 0; i < length; i++) {
    resultS += (char)payload[i];
  }

  resultS.trim();          // Elimina espacios o saltos
  resultS.toLowerCase();   // Convierte todo a minúsculas

  Serial.println(resultS);

  // Acción según el comando recibido
  if (resultS == "subir") {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    mqttClient.publish("elevador/estado", "Subiendo");

    mostrarNumero(numeros[0]);


  } else if (resultS == "bajar") {
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
    mqttClient.publish("elevador/estado", "Bajando");
    mostrarNumero(numeros[1]);


  } else if (resultS == "detener") {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW);
    mqttClient.publish("elevador/estado", "Detenido");

    mostrarNumero(numeros[2]);

  } else if (resultS == "abrir") {
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
    delay(800);
    digitalWrite(in3, LOW); digitalWrite(in4, LOW);
    mqttClient.publish("puerta/estado", "Puerta abierta");

    mostrarNumero(numeros[3]);

  } else if (resultS == "cerrar") {
    digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
    delay(800);
    digitalWrite(in3, LOW); digitalWrite(in4, LOW);
    mqttClient.publish("puerta/estado", "Puerta cerrada");

    mostrarNumero(numeros[4]);
  } else {
    mqttClient.publish("elevador/estado", "Comando inválido");
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando MQTT...");
    if (mqttClient.connect("ESP32ElevadorClient")) {
      Serial.println("Conectado");
      mqttClient.subscribe("elevador/comando");
    } else {
      Serial.print("Fallo: ");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  wifiInit();
  mqttClient.setServer(mqtt_server, port);
  mqttClient.setCallback(callback);
}

void loop() {
  if (!mqttClient.connected()) reconnect();
  mqttClient.loop();
}

void mostrarNumero(byte valor) {
  // Invertimos los bits porque el display es ÁNODO COMÚN
  byte invertido = ~valor;

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, invertido);
  digitalWrite(latchPin, HIGH);
}
