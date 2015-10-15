# NOTES

- documenation omits dependency on [MFRC522 library](https://github.com/miguelbalboa/rfid)
- install all dependencies into the Arduino IDE library location:
  - on OSX, it's in `$HOME/Documents/Arduino/libraries`
  - should look like this:

    ```bash
    Adafruit_FONA_Library/
      Adafruit_FONA.cpp
      Adafruit_FONA.h
    I2Cdev/
      I2Cdev.cpp
      I2Cdev.h
    MPU6050/
      helper_3dmath.h
      MPU6050_6Axis_MotionApps20.h
      MPU6050_9Axis_MotionApps41.h
      MPU6050.cpp
      MPU6050.h
    rfid/
      MFRC522.cpp
      MFRC522.h
    RunningAverage/
      RunningAverage.cpp
      RunningAverage.h
    ```

- also, the call to the FONA GPS location query was outdated; the Adafruit_FONA library on github has since been updated.  Changed call from `fona.getGPSlocation(gpsdata, 80);` to `fona.getGPS(0, gpsdata, 80);`

