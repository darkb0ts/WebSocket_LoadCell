
// Import required libraries

#include <HX711_ADC.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


//------------------------------------------------------------
const int HX711_dout = 23; //mcu > HX711 dout pin
const int HX711_sck = 22; //mcu > HX711 sck pin
//HX711 constructor:                                                       //load cell init()
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;
unsigned long t = 0;
//----------------------------------------------------------------------


// Variables to store temperature values
String temperatureC = "";

const char* ssid = "xxx"; //enter your ssid
const char* password = "xxx"; //enter your password
const IPAddress staticIP(192, 168, 6, 206);  //* hardcode ip address
const IPAddress gateway(192, 168, 6, 1); //* hardcore gateway
const IPAddress subnet(255, 255, 255, 0);//* hardcore subnet

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDSTemperatureC() {
  //* read load cell data using HX711_ADC.h
  long k = LoadCell.smoothedData(); //* smoothed data is get data .modifed HX711_ADC.h module for get raw data from load cell.
  return String(k); //* return load cell str data
}

//* html page for web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 Server</h2>
  <p>
    <span class="ds-labels">Load Cell Data</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, 1000) ;
</script>
</html>)rawliteral";

// Replaces placeholder with esp32 values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATUREC"){
    return temperatureC;
  }
  return String();
}

void setup(){
Serial.begin(115200);
pinMode(LED_BUILTIN, OUTPUT); // set inbuild led in esp32
  Serial.print("Connecting to ");
  Serial.println(ssid);
  LoadCell.begin();
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 106.0; // uncomment this if you want to set the calibration value in the sketch
  #if defined(ESP8266)|| defined(ESP32)
  #endif
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }

  temperatureC = readDSTemperatureC();  //* first time showing in web browser with async server

  // Connect to Wi-Fi
  WiFi.config(staticIP, gateway, subnet);        //* set manual wifi ip and gateway and subnet
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {         //* check the wifi is connect or wait for 500 sec milies second 
    Serial.print("......");
    delay(500);
  }
  Serial.println("connect");
  digitalWrite(LED_BUILTIN, HIGH);          //* if wifi is connect esp32 inbuild led on
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());             //* print ip address in serial

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){               //* main screen for web browser
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){  //* api for load cell data [/temperaturec]
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  // Start server
  server.begin();
}
 
void loop(){
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      long k = LoadCell.smoothedData(); // get raw data from load cell
      Serial.print("Raw Data : "); Serial.println(k);
      if (k>80000){
        Serial.print("your processs");
      }
      else{
         Serial.print("your processs");
      }
      temperatureC = readDSTemperatureC(); //@@ get data from load cell and update in web browser
      newDataReady = 0;
      t = millis();
    }
  }
}