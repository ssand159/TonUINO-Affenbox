import serial, time, sys
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)
time.sleep(1)
arduino.reset_input_buffer()
arduino.reset_output_buffer()
arduino.readlines()
print (arduino.readline())
sys.exit(0)
