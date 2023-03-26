import os
import glob

import cv2
import numpy as np
import serial


def get_image(ser, width, height):
    buf = ser.read(width * height * 1 + 3)
    img = np.zeros((height, width), dtype=np.uint8)
    for i in range(height):
        for j in range(width):
            img[i, j] = buf[i*height + j]
    score = [0, 0, 0]
    for k in range(3):
        score[k] = buf[i * height + j + 1 + k]
    return img, score



ITER = 0
def main():
    global ITER
    port = "/dev/ttyACM0"
    rate = 9600
    output_path = 'output'
    ser = serial.Serial(port, rate)
    width, height = 48, 48
    files = glob.glob(output_path + "/*")
    for f in files:
        os.remove(f)
    os.makedirs(output_path, exist_ok=True)

    while True:
        img, score = get_image(ser, width, height)
        #print(np.min(img), np.max(img), np.mean(img))
        print(score)

        cv2.imshow('frame', img)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        cv2.imwrite(f'output/{ITER}.png', img)
        ITER += 1

    cv2.destroyAllWindows()  # if only it was possible...


if __name__ == '__main__':
    main()
