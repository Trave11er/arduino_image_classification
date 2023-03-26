/*
  Circuit:
    - Arduino Nano 33 BLE board
    - OV7670 camera module:
      - 3.3 connected to 3.3
      - GND connected GND
      - SIOC connected to A5
      - SIOD connected to A4
      - VSYNC connected to 8
      - HREF connected to A1
      - PCLK connected to A0
      - XCLK connected to 9
      - D7 connected to 4
      - D6 connected to 6
      - D5 connected to 5
      - D4 connected to 3
      - D3 connected to 2
      - D2 connected to 0 / RX
      - D1 connected to 1 / TX
      - D0 connected to 10
*/

#include <Arduino_OV767X.h>

int bytesPerFrame;
const int width = 176;
const int height = 144;
byte data[width * height]; // QVGA: 320x240 X 1 bytes per pixel (GRAYSCALE)
const int w = 48;
const int h = 48;
byte sub_data[w * h];  // same as int8_t: https://forum.arduino.cc/t/is-uint8_t-equivalent-to-byte-etc/201618
const int width_height = 3;
const int width_height_sq = width_height * width_height;
const int width_offset = 0;
byte debug_image[width*height] = {};

void setup() {
  Serial.begin(115200);
  // uncomment to start only on serial connection
  // while (!Serial);
  // resolution, format, fps
  if (!Camera.begin(QCIF, GRAYSCALE, 1)) {
    Serial.println("Failed to initialize camera!");
    while (1);
  }

  //bytesPerFrame = Camera.width() * Camera.height() * Camera.bytesPerPixel();
  bytesPerFrame = w*h;
}

void loop() {
  Camera.readFrame(data);

  // downsample
  int min_ = 255, max_ = 0;
  float avg_ = 0;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      float sum = 0;
      for (int k = 0;  k < width_height; k++) {
        for (int l = 0; l < width_height; l++) {
          sum += (data[width_height*(width * i + j) + width * k + l + width_offset]);
        }
      }
      sum = sum / width_height_sq;

      sub_data[i * w + j] = static_cast<int8_t>(sum);
      // sub_data[i * 48 + j] = debug_image[i * 48 + j];
    }

  }

  Serial.write(sub_data, bytesPerFrame);
  byte dummy_scores[3] = {0, 0, 0};
  Serial.write(dummy_scores, 3);
}
