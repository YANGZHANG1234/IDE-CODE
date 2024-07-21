#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <SD.h>
#include <SPI.h>
#include <OneWire.h>

// 定义引脚
const int chipSelect = 17; // SD卡模块的CS引脚
const int switchPin = 16;  // 开关连接到引脚16
const int pressureSensorPin = 33; // 压力传感器连接到GPIO33
const int emgSensorPin = 25; // EMG传感器连接到GPIO25
const int mmgSensorPins[] = {36, 34, 35, 32}; // MMG传感器连接到GPIO36, GPIO34, GPIO35, GPIO32

// 定义缓冲区大小
const int bufferSize = 100000;

// 定义传感器数据缓冲区
char dataBuffer[bufferSize];
int bufferIndex = 0;

// MPU6050对象
MPU6050 mpu;

// 文件对象
File dataFile;

unsigned long startTime = 0; // 记录开始时间
unsigned long buttonPressedTime = 0; // 记录按钮按下时间
const unsigned long buttonHoldTime = 1000; // 按钮保持时间（1秒）
unsigned long lastReadTime = 0; // 上次读取数据的时间
const unsigned long readInterval = 3; // 数据读取间隔（3毫秒）

bool isRunning = false; // 程序是否运行

int fileIndex = 0; // 文件索引
char fileName[20]; // 文件名缓冲区

void createNewFile() {
  snprintf(fileName, sizeof(fileName), "/data_%d.csv", fileIndex++);
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    Serial.print("Created file: ");
    Serial.println(fileName);
    dataFile.println("Time,MMG1,MMG2,MMG3,MMG4,EMG,Pressure,AccelX,AccelY,AccelZ"); // 写入CSV头
    dataFile.close();
  } else {
    Serial.println("Error creating file");
  }
}

void writeBufferToSD() {
  dataFile = SD.open(fileName, FILE_APPEND);
  if (dataFile) {
    dataFile.write((uint8_t*)dataBuffer, bufferIndex);
    dataFile.close();
    bufferIndex = 0; // 清空缓冲区
    Serial.println("Data successfully written to SD card."); // 打印成功信息
  } else {
    Serial.println("Error opening file for writing");
  }
}

void pressButton() {
  if (digitalRead(switchPin) == LOW) {
    if (buttonPressedTime == 0) {
      buttonPressedTime = millis();
    } else if (millis() - buttonPressedTime >= buttonHoldTime) {
      isRunning = !isRunning; // 切换运行状态
      Serial.println(isRunning ? "Starting main program" : "Stopping main program");
      buttonPressedTime = 0;
      if (isRunning) {
        startTime = millis(); // 设置开始时间
        createNewFile(); // 创建新文件
      } else {
        if (bufferIndex > 0) {
          writeBufferToSD(); // 将缓冲区数据写入SD卡
        }
      }
    }
  } else {
    buttonPressedTime = 0;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // 初始化MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  // 初始化SD卡
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed");
    while (1);
  }

  // 设置开关引脚模式
  pinMode(switchPin, INPUT_PULLUP);

  // 初始化传感器引脚
  for (int i = 0; i < 4; i++) {
    pinMode(mmgSensorPins[i], INPUT);
  }
  pinMode(emgSensorPin, INPUT);
  pinMode(pressureSensorPin, INPUT);
}

void loop() {
  pressButton();

  if (isRunning) {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= readInterval) {
      lastReadTime = currentTime;

      // 读取传感器数据
      int mmgData[4];
      for (int i = 0; i < 4; i++) {
        mmgData[i] = analogRead(mmgSensorPins[i]);
      }

      int emgData = analogRead(emgSensorPin);
      int pressureData = analogRead(pressureSensorPin);

      int16_t ax, ay, az;
      mpu.getAcceleration(&ax, &ay, &az);

      // 打印传感器数据到串口
      Serial.printf("Time: %lu, MMG: %d, %d, %d, %d, EMG: %d, Pressure: %d, Accel: %d, %d, %d\n",
                    currentTime - startTime, mmgData[0], mmgData[1], mmgData[2], mmgData[3],
                    emgData, pressureData, ax, ay, az);

      // 格式化传感器数据并添加到缓冲区
      bufferIndex += snprintf(dataBuffer + bufferIndex, bufferSize - bufferIndex,
                              "%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                              currentTime - startTime, mmgData[0], mmgData[1], mmgData[2], mmgData[3],
                              emgData, pressureData, ax, ay, az);

      // 检查缓冲区是否接近满
      if (bufferIndex >= bufferSize - 100) {
        writeBufferToSD();
      }
    }
  }
}
