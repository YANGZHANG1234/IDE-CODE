#include "HX711.h"

const int dataPin = 50;       // 连接到HX711的数据引脚
const int clockPin = 52;      // 连接到HX711的时钟引脚

HX711 scale;

long offset;  // 偏移量
float scale_factor;  // 比例系数

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 称重传感器程序");

  scale.begin(dataPin, clockPin);
  
  // 首次运行时进行校准
  calibrate();

  // 使用校准后的偏移量和比例系数
  scale.set_offset(offset);
  scale.set_scale(scale_factor);

  Serial.print("偏移量: ");
  Serial.println(offset);
  Serial.print("比例系数: ");
  Serial.println(scale_factor);
  Serial.println("开始测量...");
}

void loop() {
  read_weight();
}

void calibrate() {
  Serial.println("\n\nCALIBRATION\n===========");
  Serial.println("remove all weight from the loadcell");
  while (Serial.available()) Serial.read();  // flush Serial input

  Serial.println("and press enter\n");
  while (Serial.available() == 0);  // wait for user to press enter

  Serial.println("Determine zero weight offset");
  scale.tare(20);  // average 20 measurements.
  offset = scale.get_offset();

  Serial.print("OFFSET: ");
  Serial.println(offset);
  Serial.println();

  Serial.println("place a weight on the loadcell");
  while (Serial.available()) Serial.read();  // flush Serial input

  Serial.println("enter the weight in (whole) grams and press enter");
  uint32_t weight = 0;
  while (Serial.peek() != '\n') {
    if (Serial.available()) {
      char ch = Serial.read();
      if (isdigit(ch)) {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  Serial.read();  // consume the newline character
  Serial.print("WEIGHT: ");
  Serial.println(weight);

  scale.calibrate_scale(weight, 20);
  scale_factor = scale.get_scale();

  Serial.print("SCALE:  ");
  Serial.println(scale_factor, 6);

  Serial.print("\nuse scale.set_offset(");
  Serial.print(offset);
  Serial.print("); and scale.set_scale(");
  Serial.print(scale_factor, 6);
  Serial.println(");\n");
  Serial.println("Calibration complete.");
}

void read_weight() {
  if (scale.is_ready()) {
    float weight = scale.get_units(10);  // 取10次平均值
    Serial.print("当前重量: ");
    Serial.print(weight, 2);  // 显示两位小数
    Serial.println(" g");
  } else {
    Serial.println("等待HX711准备好...");
  }

  delay(500);  // 每0.5秒读取一次重量
}
