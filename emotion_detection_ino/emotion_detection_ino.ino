/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <TensorFlowLite.h>
#include <Arduino_OV767X.h>

#include "main_functions.h"

#include "detection_responder.h"
#include "model_settings.h"
#include "emotion_detect_model_data.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

// In order to use optimized tensorflow lite kernels, a signed int8_t quantized
// model is preferred over the legacy unsigned model format. This means that
// throughout this project, input images must be converted from unisgned to
// signed format. The easiest and quickest way to convert from unsigned to
// signed 8-bit integers is to subtract 128 from the unsigned value to get a
// signed value.

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 140 * 1024;  // Determined by trial and error
static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

const int width = 176;
const int height = 144;
const int h = 48;
const int w = 48;
const int width_height = 3;
const int width_height_sq = width_height * width_height;
const int width_offset = 0;
byte data[width * height]; // Receiving QCIF grayscale from camera = 176 * 144 * 1
byte sub_data[h * w];
byte debug_image[width * height] = {};

// The name of this function is important for Arduino compatibility.
void setup() {
  Serial.begin(115200);
  // uncomment to start only on serial connection
  // while (!Serial);
  // resolution, format, fps
  if (!Camera.begin(QCIF, GRAYSCALE, 5)) {
    //Serial.println("Failed to initialize camera!");
    while (1);
  }
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_emotion_detect_model_data);

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<6> micro_op_resolver;
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();

  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    return;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);
}


// Get an image from the camera module
bool GetImage(int image_width, int image_height, int channels, float* image_data) {
  // Read camera data
  for (int i=0; i < 1; i++) {
    Camera.readFrame(data);
  }
  // downsample
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      float sum = 0;
      for (int k = 0;  k < width_height; k++) {
        for (int l = 0; l < width_height; l++) {
          sum += (data[width_height*(width * i + j) + width * k + l + width_offset]);
        }
      }
      sum = sum / width_height_sq;
      image_data[i * 48 + j] = sum;
      //image_data[i * 48 + j] = debug_image[i * 48 + j];
      sub_data[i * 48 + j] = static_cast<int8_t>(sum);
    }
  }
  Serial.write(sub_data, w*h);
  return true;
}

// The name of this function is important for Arduino compatibility.
void loop() {
  // Get image from provider.

  if (!GetImage(kNumCols, kNumRows, kNumChannels, input->data.f)) {
    //Serial.print("Image capture failed.");
  }

  delay(1000);
  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter->Invoke()) {
    //Serial.print("Invoke failed.");
  }

  // Process the inference results.
  TfLiteTensor* output = interpreter->output(0);

  float frown_score = output->data.f[kFrownIndex];
  float happy_score = output->data.f[kHappyIndex];
  float neutral_score = output->data.f[kNeutralIndex];
  RespondToDetection(frown_score, happy_score, neutral_score);

}
