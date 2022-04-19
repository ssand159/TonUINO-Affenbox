import serial, time, sys
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)
arduino.reset_input_buffer()
arduino.reset_output_buffer()
arduino.close()
arduino.open()
print (arduino.readlines())
  
sys.exit(0)
