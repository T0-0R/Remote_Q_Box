#include <WiFiS3.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Button.h>
#include <string>

const char* ssid = "xxxxxx";
const char* password = "12345678";
const char* mqtt_server = "10.42.0.1";

String input_ssid = "";
String input_pass = "";
String input_mqtt_server = "";

int led = LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4);
int currentAlphabet = 48;
unsigned long Bt3_Time_press;
const int relayPin1 = 8;   // Connect the signal pin of relay 1 to digital pin 2
const int relayPin2 = 9;   // Connect the signal pin of relay 2 to digital pin 3
const int relayPin3 = 10;  // Connect the signal pin of relay 3 to digital pin 4
const int relayPin4 = 11;
const int sensorPin1 = A0;
const int sensorPin2 = A1;
const int numReadings = 5;   // Number of readings to use for the moving average
int readings1[numReadings];  // Array to store the last 10 readings for sensor 1
int readings2[numReadings];  // Array to store the last 10 readings for sensor 2
int index1 = 0;              // Index for the circular buffer of sensor 1
int index2 = 0;              // Index for the circular buffer of sensor 2
int menu = 1;
int menu2 = 1;
bool flag = true;
bool flag2 = true;
bool flag_m1 = false;
bool status_wifi = false;
bool status_mqtt = false;
unsigned long lastSensorCheckTime = 0;          // Variable to store the last time sensor check was performed
const unsigned long sensorCheckInterval = 500;  // Interval in milliseconds
const String version = "v3";

IPAddress ip(172, 30, 39, 62);
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
String strAverageReading1_ = "0";
String strAverageReading2_ = "0";
unsigned long previousMillis = 0;
bool Sensor1_ststus = false;
bool Sensor2_ststus = true;
bool publishSuccess = false;
bool error_status = false;

Button button0(2);
Button button1(3);
Button button2(4);
Button button3(5);

void setup() {
  // put your setup code here, to run once:
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

  // Read the data from EEPROM during setup
  readDataFromEEPROM(input_ssid, 0);
  readDataFromEEPROM(input_pass, input_ssid.length() + 1);
  readDataFromEEPROM(input_mqtt_server, input_pass.length() + input_ssid.length() + 2);
  readDataFromEEPROM(data1, input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 3);
  readDataFromEEPROM(mqtt_sensor1_topic, data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 4);
  readDataFromEEPROM(data2, mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 5);
  readDataFromEEPROM(mqtt_sensor2_topic, data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 6);
  readDataFromEEPROM(mqtt_bt_topic, mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 7);
  readDataFromEEPROM(mqtt_check_topic, mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 8);
  readDataFromEEPROM(mode, mqtt_check_topic.length() + mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 9);

  Serial.print("input_ssid: ");
  Serial.println(input_ssid);
  Serial.print("input_pass: ");
  Serial.println(input_pass);
  Serial.print("input_mqtt_server: ");
  Serial.println(input_mqtt_server);

  if (input_ssid.length() > 0 && input_pass.length() > 0) {
    setup_wifi(input_ssid.c_str(), input_pass.c_str());  // Call WiFi setup function
    // setup_wifi(ssid, "password");
  } else {
    setup_wifi(ssid, password);  // Call WiFi setup function
  }

  client.setServer(input_mqtt_server.c_str(), 1884);
  client.setCallback(callback);

  // Initialize the arrays with zeros
  for (int i = 0; i < numReadings; i++) {
    readings1[i] = 0;
    readings2[i] = 0;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (status_wifi && status_mqtt) {
    if (currentMillis - previousMillis >= 3000) {
      previousMillis = currentMillis;  // Save the last time the display was updated
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Remote Q is Ready.");
      digitalWrite(relayPin1, HIGH);
      digitalWrite(relayPin3, LOW);
      lcd.setCursor(1, 2);
      lcd.print("Status WiFi: ");
      lcd.print(status_wifi ? "True" : "False");
      lcd.setCursor(1, 3);
      lcd.print("Status MQTT: ");
      lcd.print(status_mqtt ? "True" : "False");
    }
  } else {
    if (currentMillis - previousMillis >= 3000) {
      previousMillis = currentMillis;  // Save the last time the display was updated
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Remote Q NOT Ready.");
      lcd.setCursor(1, 2);
      lcd.print("Status WiFi: ");
      lcd.print(status_wifi ? "True" : "False");
      lcd.setCursor(1, 3);
      lcd.print("Status MQTT: ");
      lcd.print(status_mqtt ? "True" : "False");
      digitalWrite(relayPin1, LOW);
      digitalWrite(relayPin3, HIGH);
      reconnect();
    }
  }

  if (millis() - lastSensorCheckTime >= sensorCheckInterval) {
    CheckSensor();
    lastSensorCheckTime = millis();
  }

  if (button0.pressed()) {
    Serial.println("Button pressed");
  } else if (button0.released()) {
    if (mode == "2" || mode == "1") {
      client.publish(mqtt_bt_topic.c_str(), data1.c_str());
    }

    Serial.println("Button released");
    for (int i = 0; i < 5; i++) {
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
        if (menu == 6) {
          lcd.clear();
          menu = 1;
        }
        delay(100);
      }
    }
  }

  if (error_status) {
    digitalWrite(relayPin3, HIGH);
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
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
    case 6:
      action6();
      break;
  }
}

void action1() {  // change ADD1, mqtt_sensor1_topic
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
            saveDataToEEPROM(data1, input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 3);
            saveDataToEEPROM(mqtt_sensor1_topic, data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 4);
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

void action2() {  // change ADD2, mqtt_sensor2_topic
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
            saveDataToEEPROM(data2, mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 5);
            saveDataToEEPROM(mqtt_sensor2_topic, data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 6);
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
                      + n_data.length(),
                    3);
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
            saveDataToEEPROM(mqtt_bt_topic, mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 7);
            delay(100);
            saveDataToEEPROM(mqtt_check_topic, mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 8);
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
      saveDataToEEPROM(mode, mqtt_check_topic.length() + mqtt_bt_topic.length() + mqtt_sensor2_topic.length() + data2.length() + mqtt_sensor1_topic.length() + data1.length() + input_mqtt_server.length() + input_pass.length() + input_ssid.length() + 9);
      flag2 = false;
      delay(1000);
      lcd.clear();
      updateMenu();
    }
  }
}

void action5() {
  WiFi.disconnect();  // Disconnect from any existing WiFi connection
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Access Point");
  lcd.setCursor(1, 1);
  lcd.print("SSID: Remote_q_box");
  lcd.setCursor(1, 2);
  lcd.print("PASSWORD: 12345678");
  lcd.setCursor(1, 3);
  lcd.print("IP: 192.48.56.2");

  char ssid[] = "Remote_q_box";  // SSID of the access point
  char pass[] = "12345678";      // Password of the access point
  int status = WL_IDLE_STATUS;
  String ssid_input = "";  // Variable to store the input SSID
  String pass_input = "";
  String mqtt_server_input = "";
  IPAddress ip(192, 48, 56, 2);

  WiFi.config(ip);
  // Create access point
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // Handle failure, maybe blink an LED or something
    return;  // Exit the function
  }

  // Access point created successfully
  Serial.println("Access Point created successfully");

  // Start the web server
  WiFiServer server(80);
  server.begin();

  // Print the IP address to Serial Monitor
  Serial.print("Access Point IP address: ");
  Serial.println(WiFi.softAPIP());

  // Main loop
  while (true) {
    // Check if button3 is pressed to shut down access point
    if (button3.pressed()) {
      Serial.println("Shutting down access point...");
      WiFi.disconnect();
      Serial.println("Access point shut down");
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print(" Access point shut down ");
      updateMenu();
      return;  // Exit the function
    }

    // Check if a client has connected
    WiFiClient client = server.available();
    if (client) {
      // Wait until the client sends some data
      while (!client.available()) {
        delay(1);
      }
      // Read the first line of the request
      String request = client.readStringUntil('\r');
      Serial.println(request);
      client.flush();

      // Send a standard HTTP response header
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println();

      // HTML content for the web page
      client.println("<!DOCTYPE html><html><head><title>Configuration</title></head><body>");
      client.println("<h1>Configuration</h1>");
      client.println("<form method='get'>");
      client.println("<label for='ssid'>SSID:</label><br>");
      client.println("<input type='text' name='ssid' placeholder='Enter SSID'><br>");
      // client.println("<label for='password'>Password: (If you want to enter! Use *factorial)<br>");
      client.println("<label for='password'>Password: <span style='color: red;'>(If you want to enter '!' Use '*factorial')</span><br>");
      client.println("<input type='password' name='pass' placeholder='Enter Password'><br>");
      client.println("<label for='mqtt_server'>MQTT Server:</label><br>");
      client.println("<input type='text' name='mqtt_server' placeholder='Enter MQTT Server Address'><br>");
      client.println("<input type='submit' value='Submit'>");
      client.println("</form>");
      client.println("</body></html>");
      
      // Handle form submission action
      if (request.startsWith("GET /?")) {
        int paramsStartIndex = request.indexOf('?') + 1;
        int paramsEndIndex = request.indexOf(' ', paramsStartIndex);
        String params = request.substring(paramsStartIndex, paramsEndIndex);

        // Split parameters at '&' symbol
        int separatorIndex1 = params.indexOf('&');
        ssid_input = urlDecode(params.substring(params.indexOf('=') + 1, separatorIndex1));
        Serial.println(ssid_input);

        // Find the password parameter
        int passIndex = params.indexOf("pass=");
        if (passIndex != -1) {
          pass_input = urlDecode(params.substring(passIndex + 5));  // Skip "pass="
          pass_input.replace("*factorial", "!"); // Replace *factorial with !
          // Trim the "pass=" prefix if present
          int passEndIndex = pass_input.indexOf('&');
          if (passEndIndex != -1) {
            pass_input = pass_input.substring(0, passEndIndex);
          }
        }

        // Find the MQTT server parameter
        int mqttServerIndex = params.indexOf("mqtt_server=");
        if (mqttServerIndex != -1) {
          mqtt_server_input = urlDecode(params.substring(mqttServerIndex + 12));  // Skip "mqtt_server="
        }

        // Save SSID, Password, and MQTT server to EEPROM if they are not blank
        if (ssid_input.length() > 0 && pass_input.length() > 0 && mqtt_server_input.length() > 0) {
          // Print the SSID on the terminal
          Serial.print("SSID submitted: ");
          Serial.println(ssid_input);
          input_ssid = ssid_input;
          Serial.print("Password submitted: ");
          Serial.println(pass_input);
          input_pass = pass_input;
          Serial.print("MQTT Server submitted: ");
          Serial.println(mqtt_server_input);

          saveDataToEEPROM(ssid_input, 0);
          saveDataToEEPROM(pass_input, ssid_input.length() + 1);
          saveDataToEEPROM(mqtt_server_input, pass_input.length() + ssid_input.length() + 2);
        }
      }

      // Close the connection
      client.stop();
    }
  }
}

void action6() {
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
      lcd.print("> 4.Select Mode");
      lcd.setCursor(0, 2);
      lcd.print(" 5.Edit Wifi");
      lcd.setCursor(0, 3);
      lcd.print(" 6.Back");
      break;
    case 5:
      lcd.clear();
      lcd.print("    Select Menu");
      lcd.setCursor(0, 1);
      lcd.print(" 4.Select Mode");
      lcd.setCursor(0, 2);
      lcd.print("> 5.Edit Wifi");
      lcd.setCursor(0, 3);
      lcd.print(" 6.Back");
      break;
    case 6:
      lcd.clear();
      lcd.print("    Select Menu");
      lcd.setCursor(0, 1);
      lcd.print(" 4.Change Topic");
      lcd.setCursor(0, 2);
      lcd.print(" 5.Edit Wifi");
      lcd.setCursor(0, 3);
      lcd.print("> 6.Back");
      break;
    case 7:
      menu = 6;
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

void callback(char* topic, byte* payload, unsigned int length) {
  if (strncmp((char*)payload, "success", length) == 0) {
    publishSuccess = true;
  } else if (strncmp((char*)payload, "error_true", length) == 0) {
    error_status = true;
  } else if (strncmp((char*)payload, "error_false", length) == 0) {
    error_status = false;
    digitalWrite(relayPin2, LOW);
    digitalWrite(relayPin3, LOW);
  } else {
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

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}


String urlDecode(String input) {
  String decoded = "";
  char a, b;
  for (size_t i = 0; i < input.length(); i++) {
    if (input[i] == '%') {
      a = input[++i];
      b = input[++i];
      char decodedChar = char(int(strtol(&a, NULL, 16)) * 16 + int(strtol(&b, NULL, 16)));
      decoded += decodedChar;
    } else if (input[i] == '+') {
      decoded += ' ';
    } else {
      decoded += input[i];
    }
  }
  return decoded;
}

void setup_wifi(const char* SSID, const char* PASS) {
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin3, HIGH);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Remote Q NOT Ready.");
  lcd.setCursor(1, 2);
  lcd.print("Status WiFi: ");
  lcd.print("Conn.");
  lcd.setCursor(1, 3);
  lcd.print("Status MQTT: ");
  lcd.print(status_mqtt ? "True" : "False");

  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin3, HIGH);
  Serial.print("SSID: ");
  Serial.println(SSID);
  Serial.print("PASS: ");
  Serial.println(PASS);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(SSID, PASS);
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);  // Wait for connection
    Serial.print(".");
    attempts++;

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
          if (menu == 6) {
            // lcd.clear();
            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Remote Q NOT Ready.");
            lcd.setCursor(1, 2);
            lcd.print("Status WiFi: ");
            lcd.print("Conn.");
            lcd.setCursor(1, 3);
            lcd.print("Status MQTT: ");
            lcd.print(status_mqtt ? "True" : "False");
            menu = 1;
          }
          delay(100);
        }
      }
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    status_wifi = true;
    Serial.println("WiFi connected");
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Remote Q NOT Ready.");
    lcd.setCursor(1, 2);
    lcd.print("Status WiFi: ");
    lcd.print(status_wifi ? "True" : "False");
    lcd.setCursor(1, 3);
    lcd.print("Status MQTT: ");
    lcd.print(status_mqtt ? "True" : "False");
  }

  if (WiFi.status() != WL_CONNECTED) {
    status_wifi = false;
    Serial.println("WiFi not connected");
  }
}

void reconnect() {

  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin3, HIGH);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Remote Q NOT Ready.");
  lcd.setCursor(1, 2);
  lcd.print("Status WiFi: ");
  lcd.print(status_wifi ? "True" : "False");
  lcd.setCursor(1, 3);
  lcd.print("Status MQTT: ");
  lcd.print("Conn.");

  status_mqtt = false;
  // Loop until we're reconnected
  while (!client.connected()) {

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Remote Q NOT Ready.");
    lcd.setCursor(1, 2);
    lcd.print("Status WiFi: ");
    lcd.print(status_wifi ? "True" : "False");
    lcd.setCursor(1, 3);
    lcd.print("Status MQTT: ");
    lcd.print("Conn.");

    if (WiFi.status() == WL_CONNECTED) {
      status_wifi = true;
      Serial.println("WiFi connected");
    }

    if (WiFi.status() != WL_CONNECTED) {
      status_wifi = false;
      Serial.println("WiFi not connected");
      setup_wifi(input_ssid.c_str(), input_pass.c_str());
    }

    if (button3.read() == Button::PRESSED) {
      delay(2000);
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
          if (menu == 6) {
            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Remote Q NOT Ready.");
            lcd.setCursor(1, 2);
            lcd.print("Status WiFi: ");
            lcd.print(status_wifi ? "True" : "False");
            lcd.setCursor(1, 3);
            lcd.print("Status MQTT: ");
            lcd.print("Conn.");
            menu = 1;
          }
          delay(100);
        }
      }
    }

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      client.subscribe(mqtt_check_topic.c_str());
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
  status_mqtt = true;
}

void CheckSensor() {
  int sensorReading1 = analogRead(sensorPin1);
  int sensorReading2 = analogRead(sensorPin2);

  // Serial.print(sensorReading1);
  // Serial.print(",");
  // Serial.println(sensorReading2);

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
  if (averageReading1 > 10) {
    averageReading1 = 1;
  } else {
    averageReading1 = 0;
  }
  if (averageReading2 > 10) {
    averageReading2 = 1;
  } else {
    averageReading2 = 0;
  }

  String strAverageReading1 = String(averageReading1);
  String strAverageReading2 = String(averageReading2);

  // Serial.print(strAverageReading1);
  // Serial.print(":");
  // Serial.println(strAverageReading2);

  if (mode == "1") {
    if (strAverageReading1_ != strAverageReading1 || strAverageReading2_ != strAverageReading2) {
      client.publish(mqtt_sensor1_topic.c_str(), (strAverageReading1 + ":" + strAverageReading2).c_str());
    }


  } else if (mode == "2") {
    if (strAverageReading1_ != strAverageReading1) {
      client.publish(mqtt_sensor1_topic.c_str(), strAverageReading1.c_str());
    }
  }
  strAverageReading1_ = strAverageReading1;
  strAverageReading2_ = strAverageReading2;
}