/*
 * Humidity and Temperature by using DHT 11 sensor.
 *
 * Circuit Diagram
 * ===============
 * Arduino +5V <---> DHT 11 PIN VCC
 * Arduino D8  <---> DHT 11 PIN DATA
 * Arduino GND <---> DHT 11 PIN GND
 *
 */


#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x50, 0x01, 0xBB, 0xFC, 0x87, 0xC8 }; // <------- PUT YOUR MAC Address Here
byte ip[] = { 192, 168, 0, 99 };       //               <------- PUT YOUR IP Address Here
byte gateway[] = { 192, 168, 0, 1 };   //               <------- PUT YOUR ROUTERS IP Address to which your shield is connected Here
byte subnet[] = { 255, 255, 255, 0 };  //               <------- It will be as it is in most of the cases
EthernetServer server(46);             //               <------- It's Defaulf Server Port for Ethernet Shield

String readString;
String STOP = "?motorOff";

//PIR start
//the time we give the sensor to calibrate
int calibrationTime = 4;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 5000;

boolean lockLow = true;
boolean takeLowTime;
int nPIR_detect;

#define LED  7
#define PIR  9
//PIR end

//DHT_11 start
#define DEBUG
#define DHT_11_PIN  8

// define DHT signals in micro seconds
#define DHT_11_START_SIG     25000
#define DHT_11_WAIT_RESP     (40 + 10)
#define DHT_11_RESP_HIGH     (80 + 10)
#define DHT_11_RESP_LOW      (80 + 10)
#define DHT_11_TRANS_LOW     (50 + 10)
#define DHT_11_DATA_HIGH     70
#define DHT_11_DATA_LOW      (28 + 10)
#define DHT_11_COMP_LOW      50

// define ERROR codes
#define RESP_TIMEOUT_ERROR     1
#define START_TIMEOUT_ERROR    2
#define TRANS_TIMEOUT_ERROR    3
#define DATA_TIMEOUT_ERROR     4
#define CHK_SUM_ERROR          5

byte DHT_11_data[5];
unsigned long PW;
unsigned long PWs[40];
//DHT_11 end

// Light sensor start
const int analogInPin = A0;
const int analogOutPin = 6;
int sensorValue = 0;
int outputValue = 0;
// Light sensor end

//Sonic sensor start
int trig = 2;
int echo = 3;
int time = 0;
int distance = 0;
//sonic sensor end

//motor start
int motorPin1 = 4;  // pin 7 on L293D IC
int motorPin2 = 5;  // pin 2 on L293D IC
int enablePin = 12; // pin 1 on L293D IC
int state = 0;
int flag = 0;       //makes sure that the serial only prints once the state
//motor end

void setup()
{
  //Ethernet start
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();

  //enable serial data print

  Serial.begin(9600);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  //pir start
  { pinMode(PIR, INPUT);     // set PIR pin input mode
    digitalWrite(PIR, LOW);
    pinMode(LED, OUTPUT);    // set LED pin output mode

    //give the sensor some time to calibrate
    Serial.print("calibrating sensor ");
    for (int i = 0; i < calibrationTime; i++) {
      Serial.print(".");
      delay(100);
    }
    Serial.println(" done");
    Serial.println("PIR SENSOR ACTIVE");
    delay(50);
    nPIR_detect = 0;
  } //pir end

  // DHT-11 start
  // set DHT-11 to Low Power Consumption Mode
  { Serial.begin(9600);
    pinMode(DHT_11_PIN, INPUT_PULLUP);
  }  // DHT-11 end

  // sonic start
  { pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    pinMode(8, OUTPUT);
    Serial.begin(9600);
  }  // sonic end

  // motor start
  // sets the pins as outputs:
  { pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(enablePin, OUTPUT);

    digitalWrite(enablePin, HIGH);  // sets enablePin high so that motor can turn on:
    Serial.begin(9600);            // initialize serial communication at 9600 bits per second:
  }  // motor end
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// setup end
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// setup end

void loop() {

  //serial monitor mark ONLY
  PIR_sensor();
  Light_sensor();
  DHT_sensor();
  Sonic_sensor();

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
        }

        //if HTTP request has ended
        if (c == '\n') {
          ///////////////
          Serial.println(readString); //print to serial monitor for debuging

          /* Start OF HTML Section. Here Keep everything as it is unless you understands its working */
          // send a standard http response header
          // now output HTML data starting with standart header
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: text/html");
          client.println();
          //client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 3"); // refresh the page automatically every 3 sec
          client.println("<HTML>");
          client.println("<HEAD>");
          //client.println("<meta http-equiv=\"refresh\" content=\"5\">"); // refresh the page automatically every 5 sec
          client.print(F("<title>My Home Control</title>"));
          client.println("<center>");
          client.println("<br />");
          client.println("<p><font size=15px> <<< My Home Automation Controller >>> </font></p>");
          client.println("</HEAD>");
          // client.println(F("<body style=\"background-color:#3b5998\">"));
          // client.println("<body bgcolor=\"#D0D0D0\">");
          client.println("<br /><br /><br />");

          //------------------------------------------- Sensor -------------------------------------------//

          //------------------------------------------- PIR -------------------------------------------//
          client.println("<p><font size=15px>Motion Detector</font></p>");
          client.print("<font size=9px>");
          if (digitalRead(PIR) == HIGH) {
            digitalWrite(LED, HIGH);
            delay(5000);
            client.println("Motion Detected! - <font color='red'>Siren On</font> -");
          } // if motion detected, then turn on LED
          else
          {
            digitalWrite(LED, LOW);
            client.println("Safe Now");
          }
          client.print("</font>");
          client.println("<br /><br />");
          client.println("<hr />");   //  --------------------- line
          client.println("<br />");   //  |    | spacebar

          //------------------------------------------- Light -------------------------------------------//
          client.print("<font size=9px>The Room is </font>");
          client.print("<font size=9px>");
          if (sensorValue > 600) { // turn on LED
            client.println("DARK. - Light <font color='red'>On</font> -");
            delay(5000);
          } else {
            client.println("BRIGHT. - Light Off -");
          }
          client.print("</font>");
          client.println("<br /><br />");
          client.println("<hr />");
          client.println("<br />");

          //------------------------------------------- DHT-11 -------------------------------------------//
          if (0 == readDHT()) {
            client.print("<font size=9px>Humidity : ");
            client.print(DHT_11_data[0], DEC);
            client.print(".");
            client.print(DHT_11_data[1], DEC);
            client.print("%, Temperature : ");
            client.print(DHT_11_data[2], DEC);
            client.print(".");
            client.print(DHT_11_data[3], DEC);
            client.println("C</font>");
          } else {
            client.print("<font size=10px>Please wait...</font>");
          }
          client.println("<br /><br />");
          //----------------------------------------------------------------------------------------------//
          if (readDHT()) {
            client.println("<font size=6px>");
            if (DHT_11_data[0] > 60) {
              client.print("Very Soggy");
            } else if (DHT_11_data[0] < 40) {
              client.print("Too Air-dry");
            } else if (DHT_11_data[0] < 60) {
              client.print("Moist Air");
            } else {
              client.print(" ");
            }
            client.println("</font>");
            client.print(" AND ");
            //----------------------------------------------------------------------------------------------//
            client.println("<font size=6px>");
            if (DHT_11_data[2] > 27) {
              client.println("So Hot");
              //      client.println("<br />");
              //      client.println(" << Recommend Door OPEN >> ");
            } else if (DHT_11_data[2] < 19) {
              client.println("Too Cold");
              //      client.println("<br />");
              //      client.println(" << Recommend Door CLOSE >> ");
            } else if (DHT_11_data[2] < 27) {
              client.println("Mild Warm");
              //      client.println("<br />");
              //      client.println(" << Optimal Condition >> ");
            } else {
              client.println(" ");
            }
            client.println("</font>");
          }
          client.println("<br /><br />");
          client.println("<hr />");
          client.println("<br />");

          //------------------------------------------- Sonic -------------------------------------------//
          if (distance < 10) { // if distance < 15cm is Open Window
            client.print("<font size=6px> << The Window is Close >> </font>");
          }
          else { // close
            client.print("<font size=6px> << The Window is Open >> </font>");
          }
          client.println("<br /><br />");
          client.println("<hr />");
          client.println("<br />");

          //------------------------------------------- Sensor -------------------------------------------//

          //------------------------------------------- MOTOR -------------------------------------------//
          client.println("<p><font size=8px><a href=\"/?r\"\">+Open The Window+</a></p>");
          client.println("<p><a href=\"/?l\"\">+Close The Window+</a></p>");
          client.println("<a href=\"/?s\"\">STOP</a></font>");
          //        client.println("<a href=\"/?REFRESH\"\">REFRESH</a></font>");
          //        client.println("<input type=\"button\" value=\"Open The Window\" onClick=\"window.location='/?motorR'\">");

          if (readString.indexOf("?r") > 0) { //checks for Right
            digitalWrite(motorPin1, LOW); // set pin 2 on L293D low
            digitalWrite(motorPin2, HIGH); // set pin 7 on L293D high
            Serial.println("Right");
          }
          else {
            if (readString.indexOf("?l") > 0) { //checks for Left
              digitalWrite(motorPin1, HIGH); // set pin 2 on L293D high
              digitalWrite(motorPin2, LOW); // set pin 7 on L293D low
              Serial.println("Left");
            }
          }
          if (readString.indexOf("?s") > 0) { //checks for STOP
            digitalWrite(motorPin1, LOW); // set pin 2 on L293D high
            digitalWrite(motorPin2, LOW); // set pin 7 on L293D low
            Serial.println("Off");
          }
          //         if (readString.indexOf("?REFRESH") > 0) { //checks for REFRESH
          //           Serial.println("REFRESH");
          //         }

          readString = ""; //clearing string for next read

          //------------------------------------------- MOTOR -------------------------------------------//

          //          client.println("</body>");
          client.println("</HTML>");

          delay(1);
          client.stop(); //stopping client
          Serial.println("client disconnected");
        } // if (c == '\n')
      } // if (client.available())
    } // while (client.connected())
  } // end if (client)
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// LOOP end
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// LOOP end

void Light_sensor() {
  sensorValue = analogRead(analogInPin);
  outputValue = map(sensorValue, 0, 1023, 0, 255);
  analogWrite(analogOutPin, outputValue);

  Serial.print("Light sensor = " );
  Serial.print(sensorValue);
  Serial.print("\t output = ");
  Serial.println(outputValue);
  // delay(100);
}

void PIR_sensor() {
  // read PIR and if there is no PIR motion detected then turn off LED
  if (digitalRead(PIR) == LOW) {
    digitalWrite(LED, LOW);
  }  // if motion detected, then turn on LED
  else  {
    digitalWrite(LED, HIGH);
  }
}

void Sonic_sensor() {
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  time = pulseIn(echo, HIGH);
  distance = time / 58;

  Serial.print("time :: ");
  Serial.print(time);
  Serial.print("ms");
  Serial.print("    distance :: ");
  Serial.print(distance);
  Serial.println("cm");
}

void DHT_sensor()  {
  if (0 == readDHT()) {
    Serial.print("Humidity: ");
    Serial.print(DHT_11_data[0], DEC);
    Serial.print(".");
    Serial.print(DHT_11_data[1], DEC);
    Serial.print("%, Temperature: ");
    Serial.print(DHT_11_data[2]);
    Serial.print(".");
    Serial.print(DHT_11_data[3], DEC);
    Serial.println("C");
  }
}


// dht11
int readDHT() {
  int error = 0;
  unsigned long l_time;
  byte CHK_SUM = 0;

  do {
    // 1. send the start signal to DHT 11 by lowering about 20ms
    pinMode(DHT_11_PIN, OUTPUT);
    digitalWrite(DHT_11_PIN, LOW);
    delayMicroseconds(DHT_11_START_SIG);

    // 2. change OUTPUT mode to INPUT_PULLUP mode. DHT 11 pin 2 is internally
    //    pulled up, so it will be read as HIGH
    pinMode(DHT_11_PIN, INPUT_PULLUP);

    // 3. wait for DHT 11 response
    l_time = micros();
    while (digitalRead(DHT_11_PIN) == HIGH) {
      if ((micros() - l_time) > DHT_11_WAIT_RESP) {
        error = RESP_TIMEOUT_ERROR;
#ifdef DEBUG
        Serial.println("DHT 11 response timeout error");
#endif
        break;
      }
    };
    if (error) break;

    // 4. wait for HIGH start bit
    l_time = micros();
    while (digitalRead(DHT_11_PIN) == LOW) {
      if ((micros() - l_time) > DHT_11_RESP_LOW) {
        error = START_TIMEOUT_ERROR;
#ifdef DEBUG
        Serial.println("DHT 11 response low too long");
#endif
        break;
      }
    };
    if (error) break;

    // 5. wait for transmit start bit
    l_time = micros();
    while (digitalRead(DHT_11_PIN) == HIGH) {
      if ((micros() - l_time) > DHT_11_RESP_HIGH) {
        error = START_TIMEOUT_ERROR;
#ifdef DEBUG
        Serial.println("DHT 11 response high too long");
#endif
        break;
      }
    };
    if (error) break;

    // 6. read out 40 bits (5 bytes data : 2 bytes for humidity, 2 bytes for
    //    temperature and 1 byte checksum byte)
    for (int i = 0; i < 5; i++) {
      byte b = 0;

      for (int j = 0; j < 8; j++) {
        PW = pulseIn(DHT_11_PIN, HIGH, DHT_11_DATA_HIGH + DHT_11_TRANS_LOW);
#ifdef DEBUG
        PWs[ i * 8 + j ] = PW;
#endif
        if (PW == 0) {
          error = DATA_TIMEOUT_ERROR;
#ifdef DEBUG
          Serial.println("DHT 11 data timeout error");
#endif
          break;
        }
        else if (PW > DHT_11_DATA_LOW) b |= ( B10000000 >> j);
      }

      if (error) break;

      DHT_11_data[i] = b;
      if (i < 4)
        CHK_SUM += b;
    }
  } while (0);

  delayMicroseconds(DHT_11_COMP_LOW);
  pinMode(DHT_11_PIN, INPUT_PULLUP);

  if (error) return error;
  if (CHK_SUM != DHT_11_data[4]) {
    error = CHK_SUM_ERROR;
#ifdef DEBUG
    Serial.println("DHT 11 chksum error");
#endif
  }
#ifdef DEBUG
  for (int i = 0; i < 5; i++) {
    //   Serial.print("data[");
    //   Serial.print(i, DEC);
    //   Serial.print("]=");

    for (int j = 0; j < 8; j++) {
      //     Serial.print(PWs[i * 8 + j]);
      //     Serial.print(" ");
    }
    //   Serial.println();
  }
#endif
  return error;
}
