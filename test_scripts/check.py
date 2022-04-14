import serial, time, sys
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)
arduino.reset_input_buffer()
arduino.reset_output_buffer()
arduino.open()
time.sleep(1)
while arduino.in_waiting() > 0:
  print (arduino.readline())
  
sys.exit(0)
