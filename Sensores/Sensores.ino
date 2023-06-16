int analog_pins[] = {A0,A1,A2,A3,A4,A5,A6,A7,A8};
int input_pins[] = {22,23,24,25,26,27,28,29,30};

// 34 en proteus
#define TEMPERATURA_PELIGOROSA 69


void setup(){
    Serial3.begin(9600);
    Serial.begin(9600);
    for(uint8_t i = 0; i<9;i++){
        pinMode(input_pins[i],INPUT_PULLUP);
    }
}

char solicitud[2];
void loop(){
    if (Serial3.available()) {
        Serial3.readBytes(solicitud, 2);
        switch(solicitud[0]){
            case 'T':{
                int pin = solicitud[1] - '0';
                Serial.println(solicitud[1]);
                Serial.println(pin);
                if(analogRead(pin) >= TEMPERATURA_PELIGOROSA){
                    Serial3.print('1');
                }else{
                    Serial3.print('0');
                }
                break;
            }
            case 'B':{
                int pin = solicitud[1] - '0';
                if(digitalRead(input_pins[pin]) == HIGH){
                    Serial3.print('1');
                }else{
                    Serial3.print('0');
                }
                break;
            }
        }
    }


}
