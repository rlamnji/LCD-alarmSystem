#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int buzzer = 10;         // 버저 핀
const int alarmButtonPin = 11; // 알람 끄기 버튼 핀
const int setButtonPin = 12;   // 설정 완료 버튼 핀
const int potPin = A0;         // 가변저항 핀

bool alarmActive = false;      // 알람 상태 변수
bool alarmTriggered = false;   // 알람이 설정된 시간이 되었는지 여부
unsigned long startMillis = 0; // 프로그램 시작 시간
int initialHour = 0;           // 초기 설정된 시간 (시)
int initialMinute = 0;         // 초기 설정된 시간 (분)
int initialSecond = 0;         // 초기 설정된 시간 (초)
int currentHour = 0;           // 현재 시간 (시)
int currentMinute = 0;         // 현재 시간 (분)
int currentSecond = 0;         // 현재 시간 (초)
int setHour = 12;              // 알람 시간 (시)
int setMinute = 0;             // 알람 시간 (분)
int setSecond = 0;             // 알람 시간 (초)
int potValue = 0;              // 가변저항 값
bool timeInitialized = false;  // 현재 시간이 초기화되었는지 여부

// I2C LCD 초기화 (주소: 0x27, LCD 크기: 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 멜로디 음표 주파수
const int melody[] = {
  1047, 1568, 988, 1047, 1175, 1047, 1175, 1319, 1397,
  1319, 880, 1175, 1175, 1047, 1047, 1047, 988, 880, 988, 1047,
};

// 각 음표의 지속 시간(ms)
const int noteDurations[] = {
  300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300
};

// 멜로디 재생함수
void playMelody() {
  if (alarmActive) { // 알람이 활성화된 경우
    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
      // 버튼 상태 확인
      if (digitalRead(alarmButtonPin) == LOW) { 
        noTone(buzzer); // 소리 멈춤
        alarmActive = false; // 알람 비활성화
        alarmTriggered = false; // 알람 트리거 초기화
        startMillis = millis() - (currentHour * 3600000UL + currentMinute * 60000UL + currentSecond * 1000UL); // 현재 시간 기준으로 startMillis 설정

        Serial.println("Alarm Stopped");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Alarm Stopped");
        delay(2000); // 메시지 표시 후 대기
        lcd.clear();
        return; // 멜로디 재생 중단
      }

      tone(buzzer, melody[i]); // 멜로디 재생
      delay(noteDurations[i]); // 각 음 길이만큼 대기
      noTone(buzzer); // 소리 멈춤
      delay(50); // 간격 대기
    }
  } else {
    return; // 재생 함수 종료
  }
}

// 초기 설정
void setup() {
  lcd.init(); // LCD 초기화
  lcd.backlight(); // 백라이트 켜기
  pinMode(buzzer, OUTPUT);
  pinMode(alarmButtonPin, INPUT_PULLUP);
  pinMode(setButtonPin, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Alarm Clock Initialized");
  Serial.println("Please enter the current time in the format HH MM SS (e.g., 12 34 56):");
}

void loop() {
  int alarmButtonState = digitalRead(alarmButtonPin);
  int setButtonState = digitalRead(setButtonPin);

  // 현재 시간 초기화 (한 번만 실행)
  if (!timeInitialized) {
    if (Serial.available() > 0) {
      initialHour = Serial.parseInt();
      initialMinute = Serial.parseInt();
      initialSecond = Serial.parseInt();

      if (initialHour >= 0 && initialHour < 24 && initialMinute >= 0 && initialMinute < 60 && initialSecond >= 0 && initialSecond < 60) {
        startMillis = millis() - (initialHour * 3600000UL + initialMinute * 60000UL + initialSecond * 1000UL);
        timeInitialized = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Time Initialized:");
        lcd.setCursor(0, 1);
        if (initialHour < 10) lcd.print("0");
        lcd.print(initialHour);
        lcd.print(":");
        if (initialMinute < 10) lcd.print("0");
        lcd.print(initialMinute);
        lcd.print(":");
        if (initialSecond < 10) lcd.print("0");
        lcd.print(initialSecond);
        delay(2000);
        lcd.clear();
      } else {
        Serial.println("Invalid time format. Please enter the current time in the format HH MM SS (e.g., 12 34 56):");
      }
    }
    return;
  }

  // 현재 시간 계산
  unsigned long elapsedMillis = millis() - startMillis;
  currentHour = (elapsedMillis / 3600000UL) % 24;
  currentMinute = (elapsedMillis / 60000UL) % 60;
  currentSecond = (elapsedMillis / 1000UL) % 60;

  // LCD에 현재 시간 표시 (시:분:초)
  lcd.setCursor(0, 0);
  if (currentHour < 10) lcd.print("0");
  lcd.print(currentHour);
  lcd.print(":");
  if (currentMinute < 10) lcd.print("0");
  lcd.print(currentMinute);
  lcd.print(":");
  if (currentSecond < 10) lcd.print("0");
  lcd.print(currentSecond);

  // 가변저항 값 읽기 및 알람 시간 설정
  potValue = analogRead(potPin);
  setHour = map(potValue, 0, 1023, 0, 24);
  setMinute = map(potValue, 0, 1023, 0, 60);

  // LCD에 설정 중인 알람 시간 표시
  lcd.setCursor(0, 1);
  lcd.print("Alarm: ");
  if (setHour < 10) lcd.print("0");
  lcd.print(setHour);
  lcd.print(":");
  if (setMinute < 10) lcd.print("0");
  lcd.print(setMinute);

  // 설정 완료 버튼이 눌리면 알람 시간 저장
  if (setButtonState == LOW) {
    alarmTriggered = true;
    Serial.print("Alarm Set to: ");
    if (setHour < 10) Serial.print("0");
    Serial.print(setHour);
    Serial.print(":");
    if (setMinute < 10) Serial.print("0");
    Serial.print(setMinute);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm set to:");
    lcd.setCursor(0, 1);
    if (setHour < 10) lcd.print("0");
    lcd.print(setHour);
    lcd.print(":");
    if (setMinute < 10) lcd.print("0");
    lcd.print(setMinute);
    delay(2000);
    lcd.clear();
  }

  // 알람 시간과 현재 시간 비교
  if (currentHour == setHour && currentMinute == setMinute && currentSecond == 0 && alarmTriggered && !alarmActive) {
    alarmActive = true;
    Serial.println("Alarm is ON!");
    lcd.setCursor(0, 1);
    lcd.print("Alarm is ON!");
    playMelody();
  }


  delay(100);
}
