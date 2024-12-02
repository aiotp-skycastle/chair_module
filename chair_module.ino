#include <PinChangeInterrupt.h>

// 핀 설정
const int BUTTON_PIN1 = 2;
const int BUTTON_PIN2 = 3;
const int BUTTON_PIN3 = 4;
const int BUTTON_PIN4 = 5;
const int TEMP_SENSOR_PIN = A0;
const int TPIN = 11;
const int EPIN = 12;

volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;
volatile bool button4Pressed = false;
volatile unsigned long echo_duration = 0;
volatile float temperature = 0.0;

// 버튼 인터럽트 핸들러
void buttonISR1() {
  button1Pressed = digitalRead(BUTTON_PIN1) == LOW;  // LOW일 때 눌림
}

void buttonISR2() {
  button2Pressed = digitalRead(BUTTON_PIN2) == LOW;  // LOW일 때 눌림
}

void buttonISR3() {
  button3Pressed = digitalRead(BUTTON_PIN3) == LOW;  // LOW일 때 눌림
}

void buttonISR4() {
  button4Pressed = digitalRead(BUTTON_PIN4) == LOW;  // LOW일 때 눌림
}

// 초음파 센서 핸들러
void echoISR() {
  static unsigned long echo_begin = 0;
  static unsigned long echo_end = 0;

  if (digitalRead(EPIN) == HIGH) {
    echo_begin = micros();  // Echo 신호가 HIGH로 변할 때 시간 기록
  } else {
    echo_end = micros();
    echo_duration = echo_end - echo_begin;  // Echo 신호의 지속 시간 계산
  }
}

// 온도 센서 핸들러
/*void tempISR() {
  int readValue = analogRead(TEMP_SENSOR_PIN);
  float voltage = readValue * 5.0 / 1024;  // 전압 계산 (5V 기준)
  temperature = voltage * 100;            // 온도 변환 (10mV/°C)
}*/

void setup() {
  Serial.begin(115200);  // 라즈베리파이와의 시리얼 통신

  // 버튼 핀을 INPUT_PULLUP 모드로 설정
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);

  // 초음파 센서 핀 설정
  pinMode(TPIN, OUTPUT);
  pinMode(EPIN, INPUT);

  // 온도 센서 핀 설정
  pinMode(TEMP_SENSOR_PIN, INPUT);

  // 각 버튼에 인터럽트 설정
  attachPCINT(digitalPinToPCINT(BUTTON_PIN1), buttonISR1, CHANGE);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN2), buttonISR2, CHANGE);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN3), buttonISR3, CHANGE);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN4), buttonISR4, CHANGE);

  // Echo 핀에 인터럽트 설정
  attachPCINT(digitalPinToPCINT(EPIN), echoISR, CHANGE);

  // 온도 센서에 인터럽트 설정
  //attachPCINT(digitalPinToPCINT(TEMP_SENSOR_PIN), tempISR, CHANGE);
}

void loop() {
  // 초음파 센서 트리거 신호 전송
  digitalWrite(TPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TPIN, LOW);
  
  // 초음파 센서 거리 계산
  long duration = pulseIn(EPIN, HIGH);
  long distance = duration / 58;  // 초음파 신호의 지속 시간을 거리로 변환 (cm)

  // 4개의 버튼 중 눌린 버튼 개수 확인
  int buttonCount = button1Pressed + button2Pressed + button3Pressed + button4Pressed;


  // 온도 센서 원시 값 읽기
  int readValue = analogRead(TEMP_SENSOR_PIN);
  float voltage = readValue * 5.0 / 1024;
  temperature = voltage * 100;

  // 시리얼로 데이터 출력
  Serial.print(buttonCount);
  Serial.print(",");
  Serial.print(temperature);
  Serial.print(",");
  Serial.println(distance);

  delay(500);  
}
