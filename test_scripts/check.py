from pickle import TRUE
import serial, time, sys
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(5, GPIO.OUT) #In 1.1 Button 1
GPIO.output(5, True)
GPIO.setup(6, GPIO.OUT) #In 1.2 Button 2
GPIO.output(6, True)
GPIO.setup(13, GPIO.OUT) #In 1.3 Button 3
GPIO.output(13, True)
GPIO.setup(19, GPIO.OUT) #In 1.4 Button 4
GPIO.output(19, True)
GPIO.setup(26, GPIO.OUT) #In 2.1 Button 5
GPIO.output(26, True)
GPIO.setup(12, GPIO.OUT) #In 2.2 !Supply
GPIO.output(12, True)
GPIO.setup(16, GPIO.OUT) #In 2.3
GPIO.output(16, True)
GPIO.setup(20, GPIO.OUT) #In 2.4
GPIO.output(20, True)

arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)
arduino.reset_input_buffer()
arduino.reset_output_buffer()
arduino.close()
arduino.open()
s = ""
i = 1
while i < 70:
  s = arduino.readline()
  if s :
    s.replace("'b",'')
    s.replace("/r/n",'')
    print (s)
  i += 1

arduino.close()
sys.exit(0)
