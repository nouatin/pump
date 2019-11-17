#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>

#define portLight D0
#define smokeA0 A0
#define vibPort D6

OneWire  ds(D2);

long lastTime = 0;
long ledTime = 0;
char msg[250]; // 150

// Wifi constant and server ip !
//***********************
const char* ssid = "Amour_divin"; 
const char* password = "0123456789";
const char* mqtt_server = "172.18.0.2"; 

WiFiClient espClient;
PubSubClient client(espClient);

byte sensorInterrupt = 0;  // 0 = pin D3
// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;
volatile byte pulseCount = 0;  
float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;

unsigned long oldTime = 0;

void setup() {  
  Serial.begin(115200);
  pinMode(smokeA0, INPUT);
  pinMode(vibPort, INPUT);
  pinMode(portLight, OUTPUT);
  digitalWrite(portLight, LOW);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(sensorInterrupt, INPUT);

//  pulseCount        = 0;
//  flowRate          = 0.0;
//  flowMilliLitres   = 0;
//  totalMilliLitres  = 0;
//  oldTime           = 0;

  // The Hall-effect sensor is connected to pin D3 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println(length);    
//  if ((char)payload[0] == '0') {
//    digitalWrite(portLight, LOW);   // Turn the light on
//  } else {
//    digitalWrite(portLight, HIGH);  // Turn the light off
//  }
//  int etat = digitalRead(portLight);      
//  snprintf (msg, 250, "{\"from\":\"device\",\"temp\":{\"outSide\":%.2f,\"inSide\":%.2f},\"hum\":%.2f,\"lampStatus\":%d}",Celsius, temp, hum, etat);
//  Serial.print("Publish message: ");
//  Serial.println(msg);
//  client.publish("info/moteur", msg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish one message...
      client.publish("info/moteur", "{\"record\":\"begin !\"}");
      // resubscribtion
      client.subscribe("moteur/action");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

float dsRead();
long vibrationMeasurment();
void flowRateMeasurment();

ICACHE_RAM_ATTR void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  flowRateMeasurment();
  long now = millis();
  if(now - ledTime > 100){
    ledTime = now;
    int ledStatus = digitalRead(portLight);
    digitalWrite(portLight, !ledStatus);
  }
  if (now - lastTime >= 2000) {
    lastTime = now;  
    float temp1 = dsRead();  
    float temp2 = dsRead();
    float temperature;
    if(temp1 != temp2){
       if(temp1 == 0.00) temperature = temp2;
       else temperature = temp1;
    }
    else temperature = temp1;
    float smokeValue = analogRead(smokeA0);
    long vibration = vibrationMeasurment();
    Serial.println("");
    Serial.print("Valeur du niveau de fumee : ");
    Serial.print(smokeValue);
    Serial.println("");         
    snprintf (msg, 250, "{\"from\":\"CoulibalyPump\",\"temp\":%.2f,\"vib\":%d,\"smok\":%.2f, \"flowRate\":%d}",
      temperature, vibration, smokeValue, flowMilliLitres);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("info/moteur", msg);
  }
}

long vibrationMeasurment(){
  long measure = pulseIn(vibPort, HIGH);
  return measure;
}

float dsRead(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return 0;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return 0;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return 0;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  return celsius;
}

void flowRateMeasurment(){
       if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");  
    Serial.print(flowMilliLitres);
    Serial.print("mL/min");
    Serial.print("\t");
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.print("mL"); 
    Serial.print("\t"); 
    Serial.print(totalMilliLitres/1000);
    Serial.print("L\n");    
    
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;    
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING);
  }
}
