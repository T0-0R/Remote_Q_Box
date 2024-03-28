#include <EEPROM.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Button.h>
#include <string>
// linux

LiquidCrystal_I2C lcd(0x27, 20, 4);
int currentAlphabet = 48;
int count_connect_wifi = 0;
unsigned long Bt3_Time_press;
const int relayPin1 = 8;   // Connect the signal pin of relay 1 to digital pin 2
const int relayPin2 = 9;   // Connect the signal pin of relay 2 to digital pin 3
const int relayPin3 = 10;  // Connect the signal pin of relay 3 to digital pin 4
const int relayPin4 = 11;
const int sensorPin1 = A0;
const int sensorPin2 = A1;
int sensorReading1 = 0;
int sensorReading2 = 0;
const int numReadings = 5; // Number of readings to use for the moving average
int readings1[numReadings];  // Array to store the last 10 readings for sensor 1
int readings2[numReadings];  // Array to store the last 10 readings for sensor 2
int index1 = 0;              // Index for the circular buffer of sensor 1
int index2 = 0;              // Index for the circular buffer of sensor 2
int menu = 1;
int menu2 = 1;
bool flag = true;
bool flag2 = true;
bool flag_m1 = false;
unsigned long lastSensorCheckTime = 0;            // Variable to store the last time sensor check was performed
const unsigned long sensorCheckInterval = 500;  // Interval in milliseconds
const String version = "v2.0";

const char* ssid = "MatrixSMR100_05LPM086";
const char* password = "smrIST#20";
const char* mqtt_server = "192.168.1.103";
//IPAddress ip(172, 30, 39, 64);
//IPAddress gateway(172, 30, 36, 15);  // Replace with your gateway IP address
//IPAddress subnet(255, 255, 252, 0); // Replace with your subnet mask

WiFiClient espClient;
PubSubClient client(espClient);

String mqtt_Test_topic = "TestTopic";
String mqtt_bt_topic = "Bt1Topic";
String mqtt_sensor1_topic = "Sensor0Topic";
String mqtt_sensor2_topic = "Sensor0Topic";
String mqtt_check_topic = "CheckBt1Topic";
String data1 = "Pxi";
String data2 = "Pxi";
String mode = "1";
String n_data = "";

bool Sensor1_ststus = false;
bool Sensor2_ststus = true;
bool publishSuccess = false;
bool error_status = false;

Button button0(2);
Button button1(3);
Button button2(4);
Button button3(5);

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi...");
    count_connect_wifi += 1;
    if (count_connect_wifi >= 20) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(" Can't Connect WiFi");
      digitalWrite(relayPin3, HIGH);
    }
  }
  count_connect_wifi = 0;
  //  Serial.println("WiFi connected");
  lcd.setCursor(0, 1);
  lcd.print("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("WiFi connected");
    lcd.setCursor(0, 2);
    lcd.print("try connect mqtt");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      //      Serial.println("MQTT connected");
      lcd.setCursor(0, 2);
      lcd.print("MQTT connected");
      // client.subscribe("CheckBt1Topic");
      client.subscribe(mqtt_check_topic.c_str());
      lcd.clear();
      lcd.print("      Mode" + mode);
      lcd.setCursor(1, 2);
      lcd.print("Remote Q is Ready");
      lcd.setCursor(16, 0);
      lcd.print(version);
      digitalWrite(relayPin1, HIGH);
      digitalWrite(relayPin3, LOW);
      client.publish(("status" + mqtt_bt_topic).c_str(), "true");
    } else {
      digitalWrite(relayPin1, LOW);
      digitalWrite(relayPin3, HIGH);
      delay(5000);
    }
  }
}

// void callback(char* topic, byte* payload, unsigned int length) {
//   const char* topicStr = mqtt_subscribe_topic.c_str();
//   if (strcmp(topic, topicStr) == 0) {
//     Serial.print("Message received on topic: ");
//     Serial.println(topic);

//     Serial.print("Payload: ");
//     for (unsigned int i = 0; i < length; i++) {
//       Serial.print((char)payload[i]);
//     }
//     Serial.println();
//     publishSuccess = true;
//   }
// }
void callback(char* topic, byte* payload, unsigned int length) {
  //  Serial.println("Message received: ");
  //  Serial.println(topic);

  // ตรวจสอบว่า payload มีค่า "success" หรือไม่
  if (strncmp((char*)payload, "success", length) == 0) {
    //    Serial.println("Publish successful!");
    publishSuccess = true;
  } else if (strncmp((char*)payload, "error_true", length) == 0) {
    error_status = true;
  } else if (strncmp((char*)payload, "error_false", length) == 0) {
    error_status = false;
    digitalWrite(relayPin2, LOW);
    digitalWrite(relayPin3, LOW);
    //    OffFullLamp();
  }
  else {
    //    Serial.println("Publish failed!");
  }
}



void saveDataToEEPROM(String& data, int address) {
  int length = data.length();
  EEPROM.write(address, length);

  for (int i = 0; i < length; ++i) {
    EEPROM.write(address + i + 1, data[i]);
  }
}

void readDataFromEEPROM(String& data, int address) {
  int length = EEPROM.read(address);
  data = "";

  for (int i = 0; i < length; ++i) {
    data += char(EEPROM.read(address + i + 1));
  }
}

void executeAction() {
  switch (menu) {
    case 1:
      action1();
      break;
    case 2:
      action2();
      break;
    case 3:
      action3();
      break;
    case 4:
      action4();
      break;
    case 5:
      action5();
      break;
  }
}

void action1() { // change ADD1, mqtt_sensor1_topic
  lcd.clear();
  lcd.print("Old ADD1:");
  lcd.setCursor(0, 1);
  lcd.print(" " + data1);
  lcd.setCursor(0, 2);
  lcd.print("Change new ADD1:");
  Bt3_Time_press = 0;
  while (1) {
    flag_m1 = true;
    if (button1.pressed()) {
      currentAlphabet++;
      if (currentAlphabet > 127) {
        currentAlphabet = 48;
      }
      // lcd.setCursor(1 + n_data.length(), 3);
      // lcd.print("Poi" + char(currentAlphabet));
      lcd.setCursor(1, 3);
      lcd.print("Pxi");
      lcd.setCursor(4 + n_data.length(), 3);
      lcd.print(char(currentAlphabet));
    } else if (button2.pressed()) {
      currentAlphabet--;
      if (currentAlphabet < 48) {
        currentAlphabet = 126;
      }
      // lcd.setCursor(4 + n_data.length(), 3);
      // lcd.print("Poi" + char(currentAlphabet));
      lcd.setCursor(1, 3);
      lcd.print("Pxi");
      lcd.setCursor(4 + n_data.length(), 3);
      lcd.print(char(currentAlphabet));
    }
    if (button3.toggled()) {
      if (button3.read() == Button::PRESSED) {
        Bt3_Time_press = millis();
      } else {
        if (millis() - Bt3_Time_press >= 2000) {
          if (Bt3_Time_press > 0) {
            n_data += char(currentAlphabet);
            lcd.setCursor(0, 3);
            lcd.print(" Pxi" + n_data);
            currentAlphabet = 48;
          }
        } else {
          if (n_data != "") {
            data1 = "Pxi" + n_data;
            mqtt_sensor1_topic = "Sensor" + n_data + "Topic";
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print(" Saving new ADD1 ");
            saveDataToEEPROM(data1, 0);
            saveDataToEEPROM(mqtt_sensor1_topic, data1.length() + 1);
          } else {
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print(" ADD1 not change ");
          }
          n_data = "";
          currentAlphabet = 48;
          break;
        }
      }
    }
  }
  delay(500);
  updateMenu();
}

void action2() { // change ADD2, mqtt_sensor2_topic
  lcd.clear();
  lcd.print("Old ADD2:");
  lcd.setCursor(0, 1);
  lcd.print(" " + data2);
  lcd.setCursor(0, 2);
  lcd.print("Change new ADD2:");
  Bt3_Time_press = 0;
  while (1) {
    flag_m1 = true;
    if (button1.pressed()) {
      currentAlphabet++;
      if (currentAlphabet > 127) {
        currentAlphabet = 48;
      }
      // lcd.setCursor(1 + n_data.length(), 3);
      // lcd.print("Poi" + char(currentAlphabet));
      lcd.setCursor(1, 3);
      lcd.print("Pxi");
      lcd.setCursor(4 + n_data.length(), 3);
      lcd.print(char(currentAlphabet));
    } else if (button2.pressed()) {
      currentAlphabet--;
      if (currentAlphabet < 48) {
        currentAlphabet = 126;
      }
      // lcd.setCursor(1 + n_data.length(), 3);
      // lcd.print("Poi" + char(currentAlphabet));
      lcd.setCursor(1, 3);
      lcd.print("Pxi");
      lcd.setCursor(4 + n_data.length(), 3);
      lcd.print(char(currentAlphabet));
    }
    if (button3.toggled()) {
      if (button3.read() == Button::PRESSED) {
        Bt3_Time_press = millis();
      } else {
        if (millis() - Bt3_Time_press >= 2000) {
          if (Bt3_Time_press > 0) {
            n_data += char(currentAlphabet);
            lcd.setCursor(0, 3);
            lcd.print(" Pxi" + n_data);
            currentAlphabet = 48;
          }
        } else {
          if (n_data != "") {
            data2 = "Pxi" + n_data;
            mqtt_sensor2_topic = "Sensor" + n_data + "Topic";
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print(" Saving new ADD2 ");
            saveDataToEEPROM(data2, mqtt_sensor1_topic.length() + data1.length() + 2);
            saveDataToEEPROM(mqtt_sensor2_topic, data2.length() + mqtt_sensor1_topic.length() + data1.length() + 3);
          } else {
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print(" ADD2 not change ");
          }
          n_data = "";
          currentAlphabet = 48;
          break;
        }
      }
    }
  }
  delay(500);
  updateMenu();
}

void action3() {
  lcd.clear();
  lcd.print("Old Topic:");
  lcd.setCursor(0, 1);
  lcd.print(" " + mqtt_bt_topic);
  lcd.setCursor(0, 2);
  lcd.print("Change new Topic:");
  Bt3_Time_press = 0;
  while (1) {
    flag_m1 = true;
    if (button1.pressed()) {
      currentAlphabet++;
      if (currentAlphabet > 127) {
        currentAlphabet = 48;
      }
      // lcd.setCursor(1, 3);
      // lcd.print("Sensor");
      // lcd.print(String(char(currentAlphabet)0));
      // lcd.print("Topic");
      lcd.setCursor(1, 3);
      lcd.print("Bt");
      lcd.setCursor(3 + n_data.length(), 3);
      lcd.print(char(currentAlphabet));
    } else if (button2.pressed()) {
      currentAlphabet--;
      if (currentAlphabet < 48) {
        currentAlphabet = 126;
      }
      lcd.setCursor(1, 3);
      lcd.print("Bt");
      lcd.setCursor(3
                    + n_data.length(), 3);
      lcd.print(char(currentAlphabet));
    }
    if (button3.toggled()) {
      if (button3.read() == Button::PRESSED) {
        Bt3_Time_press = millis();
      } else {
        if (millis() - Bt3_Time_press >= 2000) {
          if (Bt3_Time_press > 0) {
            n_data += String(char(currentAlphabet));
            lcd.setCursor(0, 3);
            lcd.print(" Bt" + n_data);
            currentAlphabet = 48;
          }
        } else {
          if (n_data != "") {
            //            mqtt_sensor_topic = "Sensor" + n_data + "Topic";/
            mqtt_bt_topic = "Bt" + n_data + "Topic";
            mqtt_check_topic = "CheckBt" + n_data + "Topic";
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print(" Saving new Topic ");
            saveDataToEEPROM(mqtt_bt_topic, mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + 4);
            delay(100);
            saveDataToEEPROM(mqtt_check_topic, mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + 5);
          } else {
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print(" Topic not change ");
          }
          n_data = "";
          currentAlphabet = 48;
          break;
        }
      }
    }
  }
  delay(500);
  updateMenu();
}

void action4() {
  updateMenu2();
  flag2 = true;
  while (flag2) {
    if (button1.pressed()) {
      menu2++;
      mode = "2";
      updateMenu2();
      delay(100);
    }
    if (button2.pressed()) {
      menu2--;
      mode = "1";
      updateMenu2();
      delay(100);
    }
    if (button3.pressed()) {
      lcd.clear();
      lcd.setCursor(1, 2);
      lcd.print("Select Mode " + mode);
      saveDataToEEPROM(mode, mqtt_check_topic.length() + mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + 6);
      flag2 = false;
      delay(1000);
      lcd.clear();
      updateMenu();
    }
  }
}

void action5() {
  flag = false;
  lcd.clear();
  delay(500);
}

void updateMenu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print("    Select Menu");
      lcd.setCursor(0, 1);
      lcd.print("> 1.Change ADD1 ");
      lcd.setCursor(0, 2);
      lcd.print(" 2.Change ADD2");
      lcd.setCursor(0, 3);
      lcd.print(" 3.Change Topic");
      break;
    case 2:
      lcd.clear();
      lcd.print("    Select Menu");
      lcd.setCursor(0, 1);
      lcd.print(" 1.Change ADD1");
      lcd.setCursor(0, 2);
      lcd.print("> 2.Change ADD2");
      lcd.setCursor(0, 3);
      lcd.print(" 3.Change Topic");
      break;
    case 3:
      lcd.clear();
      lcd.print("    Select Menu");
      lcd.setCursor(0, 1);
      lcd.print(" 1.Change ADD1 ");
      lcd.setCursor(0, 2);
      lcd.print(" 2.Change ADD2");
      lcd.setCursor(0, 3);
      lcd.print("> 3.Change Topic");
      break;
    case 4:
      lcd.clear();
      lcd.print("    Select Menu");
      lcd.setCursor(0, 1);
      lcd.print(" 3.Change Topic");
      lcd.setCursor(0, 2);
      lcd.print("> 4.Select Mode");
      lcd.setCursor(0, 3);
      lcd.print(" 5.Back");
      break;
    case 5:
      lcd.clear();
      lcd.print("    Select Menu");
      lcd.setCursor(0, 1);
      lcd.print(" 3.Change Topic");
      lcd.setCursor(0, 2);
      lcd.print(" 4.Select Mode");
      lcd.setCursor(0, 3);
      lcd.print("> 5.Back");
      break;
    case 6:
      menu = 5;
      break;
  }
}

void updateMenu2() {
  switch (menu2) {
    case 0:
      menu2 = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print("    Select Mode");
      lcd.setCursor(0, 1);
      lcd.print("> 1.Mode 1");
      lcd.setCursor(0, 2);
      lcd.print(" 2.Mode 2");
      break;
    case 2:
      lcd.clear();
      lcd.print("    Select Mode");
      lcd.setCursor(0, 1);
      lcd.print(" 1.Mode 1");
      lcd.setCursor(0, 2);
      lcd.print("> 2.Mode 2");
      break;
    case 3:
      menu2 = 2;
      break;
  }
}

void CheckSensor() {
  sensorReading1 = analogRead(sensorPin1);
  sensorReading2 = analogRead(sensorPin2);

  // Store the readings in the circular buffers
  readings1[index1] = sensorReading1;
  readings2[index2] = sensorReading2;
  index1 = (index1 + 1) % numReadings;
  index2 = (index2 + 1) % numReadings;

  // Calculate the moving averages
  int total1 = 0, total2 = 0;
  for (int i = 0; i < numReadings; i++) {
    total1 += readings1[i];
    total2 += readings2[i];
  }
  int averageReading1 = total1 / numReadings;
  int averageReading2 = total2 / numReadings;
  if (averageReading1 >= 5) {
    averageReading1 = 1;
  } else {
    averageReading1 = 0;
  }
  if (averageReading2 >= 5) {
    averageReading2 = 1;
  } else {
    averageReading2 = 0;
  }
  String strAverageReading1 = String(averageReading1);
  String strAverageReading2 = String(averageReading2);

  if (mode == "1") {
    client.publish(mqtt_sensor1_topic.c_str(), strAverageReading1.c_str());
    client.publish(mqtt_sensor2_topic.c_str(), strAverageReading2.c_str());
    Serial.print(strAverageReading1.c_str());
    Serial.print(" , ");
    Serial.println(strAverageReading2.c_str());
  } else if (mode == "2") {
    client.publish(mqtt_sensor1_topic.c_str(), strAverageReading1.c_str());
    Serial.println(strAverageReading1.c_str());
  }


}

void OnFullLamp() {
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);
  digitalWrite(relayPin3, HIGH);
  digitalWrite(relayPin4, HIGH);
}
void OffFullLamp() {
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin2, LOW);
  digitalWrite(relayPin3, LOW);
  digitalWrite(relayPin4, LOW);
}

void blinkRED() {
  static unsigned long previousMillis = 0;
  const long interval = 500;  // Blink interval in milliseconds

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // Save the current time
    previousMillis = currentMillis;

    // Toggle the LED state
    if (digitalRead(relayPin3) == LOW) {
      digitalWrite(relayPin3, HIGH);
    } else {
      digitalWrite(relayPin3, LOW);
    }
  }
}

void blinkYELLOW() {
  static unsigned long previousMillis = 0;
  const long interval = 500;  // Blink interval in milliseconds

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // Save the current time
    previousMillis = currentMillis;

    // Toggle the LED state
    if (digitalRead(relayPin2) == LOW) {
      digitalWrite(relayPin2, HIGH);
    } else {
      digitalWrite(relayPin2, LOW);
      OffFullLamp();
    }
  }
}

void setup() {
  Serial.begin(9600);

  button0.begin();
  button1.begin();
  button2.begin();
  button3.begin();
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(13, OUTPUT);

  lcd.init();
  lcd.backlight();

//  WiFi.config(ip);
  setup_wifi();
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);

  // Read the data from EEPROM during setup
  readDataFromEEPROM(data1, 0);
  readDataFromEEPROM(mqtt_sensor1_topic, data1.length() + 1);
  readDataFromEEPROM(data2, mqtt_sensor1_topic.length() + data1.length() + 2);
  readDataFromEEPROM(mqtt_sensor2_topic, data2.length() + mqtt_sensor1_topic.length() + data1.length() + 3);
  readDataFromEEPROM(mqtt_bt_topic, mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + 4);
  readDataFromEEPROM(mqtt_check_topic, mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + 5);
  readDataFromEEPROM(mode, mqtt_check_topic.length() + mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + 6);

  // Initialize the arrays with zeros
  for (int i = 0; i < numReadings; i++) {
    readings1[i] = 0;
    readings2[i] = 0;
  }

  reconnect();
}

void loop() {
  if (!client.connected()) {
    reconnect();
    lcd.clear();
    lcd.print("      Mode" + mode);
    lcd.setCursor(1, 2);
    lcd.print("Remote Q is Ready");
    lcd.setCursor(16, 0);
    lcd.print(version);
  }
  if (millis() - lastSensorCheckTime >= sensorCheckInterval) {
    CheckSensor();
    lastSensorCheckTime = millis();
    client.publish(("status" + mqtt_bt_topic).c_str(), "true");
  }

  if (button0.pressed()) {
    //    OnFullLamp();
    Serial.println("Button pressed");
  } else if (button0.released()) {
    //    OffFullLamp();
    if (mode == "2") {
      // if (Sensor1_ststus == false) {
      client.publish(mqtt_bt_topic.c_str(), data1.c_str());
      // }
    }

    Serial.println("Button released");
    for (int i = 0; i < 5; i++) {
      client.loop();
      Serial.print("Attempt: ");
      Serial.println(i);
      digitalWrite(relayPin2, HIGH);
      delay(100);
      digitalWrite(relayPin2, LOW);
      delay(100);
    }
    if (publishSuccess) {
      digitalWrite(relayPin2, HIGH);
      Serial.println("Message published successfully.");
    } else {
      //      digitalWrite(relayPin3, HIGH);
      //      blinkYELLOW();
      Serial.println("Publishing failed after 5 attempts.");
    }
    publishSuccess = false;
  }

  if (button3.pressed()) {
    updateMenu();
    flag = true;
    while (flag) {
      if (button1.pressed()) {
        menu++;
        updateMenu();
        delay(100);
      }
      if (button2.pressed()) {
        menu--;
        updateMenu();
        delay(100);
      }
      if (button3.pressed()) {
        executeAction();
        if (menu == 5) {
          lcd.clear();
          lcd.print("      Mode" + mode);
          lcd.setCursor(1, 2);
          lcd.print("Remote Q is Ready");
          lcd.setCursor(16, 0);
          lcd.print(version);
          menu = 1;
        }
        delay(100);
      }
    }
  }
  client.loop();

  if (error_status) {
    digitalWrite(relayPin3, HIGH);
  }
}