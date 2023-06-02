/* конектимось до вайфай мережі беремо данні із памяті та пробуємо по цим данним законектитись 
якщо данних немає переходимо у режим точки доступу та чекаємо данні через програму WiFi to Serial 
щоб передати логін для запису у паммять потрібно перед логіном вказати SSID:"ваш логін вайфай мережі"
щоб передати пароль перед паролем вводим Password:"ваш пароль вайфай мережі"
плани  на розвиток добавити скан мережі */

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiClient.h>

const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
const int maxAttempts = 5;

const int eepromSize = 512;        // виділяюм область памяті з якою будем працювати а саме від 0 до 512 байт
const int loginAddress = 2;        // адрес логіна вайфа  починається з другого байту в EEPROM
const int passwordAddress = 32;    // адрес паролю починається з 32 байту
const int lenAddress = 0;          //сюди будем записувати довжину логіну та паролю адрес 0 виділяєм під довжину логіну адрес 1 під довжину паролю 

WiFiServer server(80);             // порт для сервера
WiFiClient client;                 

bool wifiConnected = false;        // змінна у якій будемо перевіряти чи вдалось нам підключитись до вайфай мережі
String newSSID;
String newPassword;
int passLen = 0;                    // відповідно змінні під довжинну логіну та паролю
int ssidLen = 0;


void setup() {
  Serial.begin(115200);            // це для виводу в монітр інфи та пошуку помилок

  EEPROM.begin(eepromSize);  


  int attempt = 0;                 
  while (!wifiConnected && attempt < maxAttempts) {
    attempt++;
    connectToWiFi();
  }

  if (!wifiConnected) {
    startAccessPointMode();
  } else {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {

  // основний код програми

}


void connectToWiFi() {
  int lenLogin = EEPROM.read(0);
  int lenPass = EEPROM.read(1);

  delay(100);
  Serial.println("LenLogin: ");
  Serial.println(lenLogin);
  if (lenLogin != 0){
    String storedSSID = readEEPROMString(loginAddress, lenLogin);
    String storedPassword = readEEPROMString(passwordAddress, lenPass);

    Serial.println(storedSSID);
    Serial.println(storedPassword);
    if (storedSSID.length() > 0 && storedPassword.length() > 0) {
      Serial.println("Connecting to WiFi...");
      WiFi.begin(storedSSID.c_str(), storedPassword.c_str());

      int timeout = 0;
      while (WiFi.status() != WL_CONNECTED && timeout < 10000) {
        Serial.print(".");
        delay(500);
        timeout += 500;
      }

      if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
      } else {
        Serial.println("Connection failed");
      }
    }
  }
  
}

void startAccessPointMode() {
  Serial.println("Failed to connect to WiFi, starting access point mode");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP8266AP", "password");

  server.begin();

  Serial.println("Access Point mode started");
  Serial.print("SSID: ESP8266AP, Password: password");

  while (!wifiConnected) {
    // Wait for WiFi connection
    delay(1000);
    if (client = server.available()) {
      processClientData();
    }
  }

  /* Prompt user to enter new SSID and password via the chosen communication method (e.g., Wi-Fi Serial)
  // Store the entered SSID and password in EEPROM
  writeEEPROMString(loginAddress, newSSID);
  writeEEPROMString(passwordAddress, newPassword);

  EEPROM.commit();

  Serial.println("New credentials saved");
  ESP.restart(); */
}

void processClientData() {
  while (client.connected()) {
    if (client.available()) {
      String data = client.readStringUntil('\n');
      data.trim();

      if (data.startsWith("SSID:")) {
        newSSID = data.substring(5);
        newSSID.trim();
      } else if (data.startsWith("Password:")) {
        newPassword = data.substring(9);
        newPassword.trim();
        Serial.println("SSID: " + newSSID);
        Serial.println("Password: " + newPassword);
        
        saveCredentials();
        break;
      }
    }
  }
}



void saveCredentials() {
  
  
  EEPROM.write(0, newSSID.length());
  EEPROM.write(1, newPassword.length());
  delay(100);
  Serial.println("length: ");
  Serial.println(EEPROM.read(0));
  int len = 0;
  for (int i = loginAddress; i < loginAddress + newSSID.length(); i++) {
    EEPROM.write(i, newSSID.charAt(len));
    
    len++;
  }
  len = 0;
  for (int i = passwordAddress; i < passwordAddress + newPassword.length(); i++) {
    EEPROM.write(i, newPassword[len]);
    
    len++;
  }
  
  if (EEPROM.commit()) {
      Serial.println("EEPROM successfully committed");
    } else {
      Serial.println("ERROR! EEPROM commit failed");
    }
  delay(100);
  Serial.println("Credentials saved");
  ESP.restart();
}

String readEEPROMString(int address, int maxLength) {
  String value;
  for (int i = 0; i < maxLength; i++) {
    char c = EEPROM.read(address + i);
    value += c;
  }
  return value;
}



void writeEEPROMString(int address, const String& value) {
  for (int i = 0; i < value.length(); i++) {
    EEPROM.write(address + i, value[i]);
  }
}
