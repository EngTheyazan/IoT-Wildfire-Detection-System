#include "DHT.h"
#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// تعريف نوع الحساس والمنفذ الموصل به
// -------------------- DHT22 --------------------
#define DHTPIN 15        // رقم المنفذ الموصل بـ DATA
#define DHTTYPE DHT22    // نوع الحساس
DHT dht(DHTPIN, DHTTYPE);

// -------------------- MQ2 --------------------
#define MQ2_PIN 34       // المدخل التناظري الموصل بـ AO في MQ2

// -------------------- HC-SR04 --------------------
#define TRIG_PIN 2
#define ECHO_PIN 4

// ---------- PIR ----------
#define PIR_PIN 27

// ---------- MPU6050 ----------
MPU6050 mpu;

// -------------------- Buzzer & LEDs --------------------
#define BUZZER_PIN 12
#define LED_RED    14
#define LED_GREEN  26

// -------------------- LDR --------------------
#define LDR_PIN 35  // المنفذ التناظري الموصل بـ AO في حساس الضوء

// -------------------- Relay --------------------
#define RELAY_PIN 13

// -------------------- OLED --------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // إذا لم يكن هناك زر Reset مخصص
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Serial.println("(DHT22, MQ2, HC-SR04, PIR, MPU6050, Buzzer, LEDs, LDR=TSL) Sensors & OLED & Relay System Starting...");

  dht.begin(); // بدء الحساسات
  Wire.begin();      // I2C communication, SDA = 21, SCL = 22 (افتراضي في ESP32)
  mpu.initialize();  // بدء MPU6050

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // إيقاف التشغيل في البداية

  
  // MPU6050 Connection Test
  if (mpu.testConnection()) {
    Serial.println("MPU6050 متصل بنجاح");
  } 
  else {
    Serial.println("فشل في الاتصال بـ MPU6050");
  }

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // عنوان I2C للشاشة
    Serial.println(F("فشل في بدء شاشة OLED"));
    for(;;); // توقف هنا
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("بدء التشغيل...");
  display.display();
  delay(1000);
}

void loop() {
  delay(2000); // انتظر 2 ثانية بين كل قراءة

  // --- قراءة DHT22 ---
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // مئوية

  // التحقق من فشل القراءة
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("⚠️ فشل في قراءة البيانات من DHT22!");
    return;
  }
  else {
    Serial.print("الرطوبة: ");
    Serial.print(humidity);
    Serial.print(" %\t");

    Serial.print("الحرارة: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }

  // --- قراءة MQ2 --- 
  int gasValue = analogRead(MQ2_PIN);
  Serial.print("قيمة الغاز (MQ2): ");
  Serial.println(gasValue);

  // إنذار إذا كانت القيمة مرتفعة
  // ....

  // --- قراءة HC-SR04 ---
  long duration;
  float distance;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; // تحويل إلى سم

  Serial.print("المسافة (HC-SR04): ");
  Serial.print(distance);
  Serial.println(" سم");

  // --- قراءة PIR ---
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    Serial.println("👣 تم الكشف عن حركة!");
    digitalWrite(BUZZER_PIN, HIGH); // تشغيل الجرس عند الحركة
  } 
  else {
    Serial.println("🔍 لا توجد حركة.");
  }
  
  // --- قراءة بيانات MPU6050 ---
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  Serial.println("🧭 MPU6050 بيانات التسارع والدوران:");
  Serial.print("  Accel X: "); 
  Serial.print(ax);
  Serial.print(" | Y: "); 
  Serial.print(ay);
  Serial.print(" | Z: "); 
  Serial.println(az);

  Serial.print("  Gyro  X: "); 
  Serial.print(gx);
  Serial.print(" | Y: "); 
  Serial.print(gy);
  Serial.print(" | Z: "); 
  Serial.println(gz);

  // --- قراءة LDR --- 
  int lightValue = analogRead(LDR_PIN);
  Serial.print("شدة الإضاءة (LDR): ");
  Serial.println(lightValue);

  // إشعار إذا كانت الإضاءة منخفضة
  if (lightValue < 1000) {
    Serial.println("🌑 الإضاءة منخفضة!");
  }

  // --- قراءة OLED Display --- 
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.print("T: ");
  display.print(temperature);
  display.print("C H: ");
  display.print(humidity);
  display.println("%");

  display.print("Gas: ");
  display.println(gasValue);

  display.print("Dist: ");
  display.print(distance);
  display.println("cm");

  display.print("Light: ");
  display.println(lightValue);

  display.display(); // عرض البيانات

  // ----- Relay Activation Logic -----
  if (temperature > 35 || gasValue > 4000 || lightValue < 1000) {
    Serial.println("⚠️ تشغيل Relay!");
    digitalWrite(RELAY_PIN, HIGH);
  } 
  else {
    digitalWrite(RELAY_PIN, LOW);
  }

  // ----- Alarm LED/Buzzer -----
  if (gasValue > 4000) {
    Serial.println("🚨 إنذار: تركيز الغاز مرتفع!");
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  } 
  else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }

  Serial.println("-----------------------------");
}
