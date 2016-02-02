NCD4Relay relayController;

SYSTEM_MODE(AUTOMATIC);

int triggerRelay(String command);

bool tripped[4];

int debugTrips[4];

int minTrips = 5;

int inputStatus[4] = {0,0,0,0};

bool relay3TimerRunning = false;

unsigned long relay3Timer = 0;

static unsigned long durationTimer = 600000;

bool allowTimer = true;

/* This function is called once at start up ----------------------------------*/
void setup()
{
    Particle.function("controlRelay", triggerRelay);
    Serial.begin(115200);
    relayController.setAddress(0,0,0);
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    int status = relayController.readAllInputs();
    int a = 0;
    for(int i = 1; i < 9; i*=2){
        if(status & i){
            debugTrips[a]++;
            if(debugTrips[a] >= minTrips){
                if(!tripped[a]){
                    tripped[a] = true;
                    //set input trip event to true
                    String eventName = "Input_";
                    eventName+=(a+1);
                    Particle.publish(eventName, "ON");
                    inputStatus[a] = 1;
                    
                }
            }
        }else{
            debugTrips[a] = 0;
            if(tripped[a]){
                tripped[a] = false;
                //set input trip event to false
                String eventName = "Input_";
                eventName+=(a+1);
                Particle.publish(eventName, "OFF");
                inputStatus[a] = 0;
                
            }
        }
        a++;
    }
    //Check input status from array
    //Input 1 is closed so turn on relay 1
    if(inputStatus[0] == 1){
        relayController.turnOnRelay(1);
    }
    //Input 1 is open so turn off relay 1
    if(inputStatus[0] == 0){
        relayController.turnOffRelay(1);
    }
    //Inputs 1 and 2 are closed so turn on relay 2
    if(inputStatus[0] == 1 && inputStatus[1] == 1){
        relayController.turnOnRelay(2);
    }
    //Inputs 1 and 2 are open so turn off relay 2
    if(inputStatus[0] == 0 && inputStatus[1] == 0){
        relayController.turnOffRelay(2);
    }
    //Input 3 is open so allow timer to run on relay 3
    if(inputStatus[2] == 0){
        allowTimer = true;
    }
    //Input 4 is closed and Input 3 is closed so turn off relay 3 and stop here
    if(inputStatus[3] == 1 && inputStatus[2] == 1){
        relayController.turnOffRelay(3);
        return;
    }
    //Input 3 is closed, relay3Timer is not currently running and we are allowed to run the timer so start the 10 minute timer.
    if(inputStatus[2] == 1 && relay3TimerRunning == false && allowTimer == true){
        relay3Timer = millis();
        relay3TimerRunning = true;
        allowTimer = false;
    }
    //Check the current Time.
    if(relay3TimerRunning){
        //Check time
        if(millis() >= relay3Timer+durationTimer){
            relayController.turnOnRelay(3);
            relay3TimerRunning = false;
        }
    }
    
    
    
}

int triggerRelay(String command){
    if(command.equalsIgnoreCase("turnonallrelays")){
        relayController.turnOnAllRelays();
        return 1;
    }
    if(command.equalsIgnoreCase("turnoffallrelays")){
        relayController.turnOffAllRelays();
        return 1;
    }
    if(command.startsWith("setBankStatus:")){
        int status = command.substring(14).toInt();
        if(status < 0 || status > 255){
            return 0;
        }
        Serial.print("Setting bank status to: ");
        Serial.println(status);
        relayController.setBankStatus(status);
        Serial.println("done");
        return 1;
    }
    //Relay Specific Command
    int relayNumber = command.substring(0,1).toInt();
    Serial.print("relayNumber: ");
    Serial.println(relayNumber);
    String relayCommand = command.substring(1);
    Serial.print("relayCommand:");
    Serial.print(relayCommand);
    Serial.println(".");
    if(relayCommand.equalsIgnoreCase("on")){
        Serial.println("Turning on relay");
        relayController.turnOnRelay(relayNumber);
        Serial.println("returning");
        return 1;
    }
    if(relayCommand.equalsIgnoreCase("off")){
        relayController.turnOffRelay(relayNumber);
        return 1;
    }
    if(relayCommand.equalsIgnoreCase("toggle")){
        relayController.toggleRelay(relayNumber);
        return 1;
    }
    if(relayCommand.equalsIgnoreCase("momentary")){
        relayController.turnOnRelay(relayNumber);
        delay(300);
        relayController.turnOffRelay(relayNumber);
        return 1;
    }
    return 0;
}
