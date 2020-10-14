#include<Wire.h>
#include<SPI.h>
#include"SdFat.h"

// Task variables

unsigned long cpucounter, ticker, milliseconds, intervalstart, intervalstop;
unsigned long timem, intervalm;                     //MPU6050
unsigned long times, intervals;                     //SDFat
unsigned long timeb;                     //Blinker task
bool run;


// SD variables
const int chipSelect = 5;
SdFat sd;
SdFile file;
#define FILE_NAME "OUTPUT.csv"

// MPU6050 variables
const int MPU_addr = 0x68; // I2C address of the MPU-6050
const int MPU2_addr = 0x69;
int16_t ax, ay, az, bx, by, bz, count;

// ================= Setup ===================
void setup() {

  // Tasks
  intervalm = intervals = 2;
  intervalstart = 5000; // Start after 5 seconds
  intervalstop = 3600000; // Stop after an hour
  timem = times = timeb = millis();
  cpucounter = ticker = 0;
  run = false;

  pinMode(LED_BUILTIN, OUTPUT);


  // SD
  sd.begin(chipSelect, SD_SCK_MHZ(50));
  file.open(FILE_NAME, O_WRONLY | O_CREAT | O_EXCL);
  file.println("ax,ay,az,bx,by,bz");  // Headers in csv


  // MPU6050
  Wire.begin();
  Wire.setClock(400000);
  ax, ay, az, bx, by, bz = 0; // Initialize vars

  // MPU 1
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0x0B);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1A);  // CONFIG register
  Wire.write(0x01);     // DLPF_CFG LPF set to 184Hz
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1C);  // ACCEL_CONFIG reg
  Wire.write(0x18);     // AFS_SEL set to 16g
  Wire.endTransmission(true);

  // MPU 2
  Wire.beginTransmission(MPU2_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0x0B);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU2_addr);
  Wire.write(0x1A);  // CONFIG register
  Wire.write(0x01);     // DLPF_CFG LPF set to 184Hz
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU2_addr);
  Wire.write(0x1C);  // ACCEL_CONFIG reg
  Wire.write(0x18);     // AFS_SEL set to 16g
  Wire.endTransmission(true);

}


// ================= Loop ===================
void loop() {

  milliseconds = millis();
  cpucounter++;

  if (run) {
    // MPU6050 task
    if (milliseconds > (timem + intervalm))
    {
      timem += intervalm;

      // MPU 1
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr, 6, true); // request a total of 14 registers
      ax = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
      ay = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      az = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

      // MPU 2
      Wire.beginTransmission(MPU2_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU2_addr, 6, true); // request a total of 14 registers
      bx = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
      by = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      bz = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    }

    // SD task
    if (milliseconds > (times + intervals))
    {
      times += intervals;

      file.print(ax); file.print(",");
      file.print(ay); file.print(",");
      file.print(az); file.print(",");
      file.print(bx); file.print(",");
      file.print(by); file.print(",");
      file.println(bz);
    }

    // Shutdown
    if (milliseconds > (timeb + intervalstop)) {
      digitalWrite(LED_BUILTIN, LOW);
      file.close();
      while (1);
    }

  }
  else {
    if (milliseconds > (timeb + intervalstart)) {
      timeb += intervalstart;
      digitalWrite(LED_BUILTIN, HIGH);
      run = true;
    }
  }


}
