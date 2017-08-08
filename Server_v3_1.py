import socket
import sys
from _thread import *
from neopixel import *
import time
import serial
import Adafruit_PCA9685

pwm_head = Adafruit_PCA9685.PCA9685(address=0x40)
pwm_left_arm = Adafruit_PCA9685.PCA9685(address=0x41)
#pwm_right_arm = Adafruit_PCA9685.PCA9685(address=0x42)
pwm_head.set_pwm_freq(60)
pwm_left_arm.set_pwm_freq(60)
#pwm_right_arm.set_pwm_freq(60)

Names = {'head' : pwm_head, 'left' : pwm_left_arm}

HOST = ''  # Symbolic name meaning all available interfaces
PORT = 8888 # Arbitrary non-privileged port

ser = ''

LED_COUNT      = 8      # Number of LED pixels.
LED_PIN        = 18      # GPIO pin connected to the pixels (18 uses PWM!).
#LED_PIN        = 10      # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 5       # DMA channel to use for generating signal (try 5)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL    = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53
LED_STRIP      = ws.WS2811_STRIP_GRB   # Strip type and colour ordering

def colorWipe(strip, color, wait_ms=50):
	"""Wipe color across display a pixel at a time."""
	for i in range(strip.numPixels()):
		strip.setPixelColor(i, color)
		strip.show()
		time.sleep(wait_ms/1000.0)


def moove_to(new_pos):
    pwm.set_pwm(1,0,new_pos)
    pwm.set_pwm(0,0,new_pos)

strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL, LED_STRIP)
# Intialize the library (must be called once before other functions).
strip.begin()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('Socket created')

try:
    ser = serial.Serial()
    ser.port = '/dev/ttyACM0'
    ser.baudrate = 115200
    ser.open()
except:
    pass

# Bind socket to local host and port
try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
    sys.exit()

print('Socket bind complete')

# Start listening on socket
s.listen(10)
print ('Socket now listening')


# Function for handling connections. This will be used to create threads
def clientthread(conn):
    # Sending message to connected client
    conn.sendall(b'Welcome to the server. Type something and hit enter\n')  # send only takes string

    # infinite loop so that function do not terminate and thread do not end.
    while True:

        # Receiving from client
        data = conn.recv(1024)
        MyData = (data.decode("utf-8").strip())

        if (MyData[0] == "E"):
            s = MyData[1:]+"g"
            ser.write(s.encode('utf-8'))
        elif (MyData[0] == "V"):
            s = MyData[1:]+"v"
            ser.write(s.encode('utf-8'))
        elif (MyData[0] == "B"):
            # Брови
            moove_to(int(MyData[1:]))
        elif (MyData[0]=="M"):
            pwm.set_pwm(2,0,int(MyData[1:]))
        elif (MyData=="Радость"):
            print("OK")
            colorWipe(strip, Color(255, 0, 0))
        elif (MyData=="Грусть"):
            print("OK")
            colorWipe(strip, Color(0, 255, 0))
        elif (MyData=="Ровное"):
            print("OK")
            colorWipe(strip, Color(0, 0, 0))
        elif (MyData[0] == "S"):
                tmp = MyData[1:].split("/")
                tmp2 = tmp[0].split('_')
                Names[tmp2[0]].set_pwm(int(tmp2[1]),0,int(float(tmp[1])))
        reply = data
        if not data:
            break

        conn.sendall(reply)

    # came out of loop
    conn.close()


# now keep talking with the client
while 1:
    # wait to accept a connection - blocking call
    conn, addr = s.accept()
    print ('Connected with ' + addr[0] + ':' + str(addr[1]))

    # start new thread takes 1st argument as a function name to be run, second is the tuple of arguments to the function.
    start_new_thread(clientthread, (conn,))

s.close()
