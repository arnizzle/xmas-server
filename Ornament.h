
class ornament{
  public:
    int LED;
    long interval = 0;
    long previousMillis = 0;
    bool ledStatus = false;
    String ornName = "";
    
    void setInterval(void) {
//      interval = random(1000, 5000);
      interval = random(300000, 500000);
      Serial.print("Setting Interval");
    }

    void turnOn () {
      digitalWrite(LED, HIGH);
      ledStatus = true;
    }
    
    void turnOff () {
      digitalWrite(LED, LOW);
      ledStatus = true;
    }

    void Setup() {
        pinMode(LED,OUTPUT);
    }

    bool ornStatus() {
      if (digitalRead(LED)) {
        return true;
      } else {
        return false;
      }
      
    }
    
    void toggle() {
       
        Serial.println("TOGGLE");
        if (ledStatus) {
          digitalWrite(LED, LOW);
          Serial.print("LOW");
        } else {
          digitalWrite(LED, HIGH);
          Serial.print("HIGH");
        }
        ledStatus =! ledStatus;  
    }
    
    void checkStatus(void) {
//      doLog("Checking Status...") ;
      
      // save the last time you blinked the LED 
      unsigned long currentMillis = millis();

      if(currentMillis - previousMillis  > interval) {
        previousMillis = currentMillis; 
        
        toggle();
        setInterval();
    }
  }
};



