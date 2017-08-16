import socket
import sys
import time
import serial
from _thread import *

INIT_STATUS = []

Names = {}

try:
        pwm_head = Adafruit_PCA9685.PCA9685(address=0x40)
        pwm_head.set_pwm_freq(60)
        Names['head'] = pwm_head
        INIT_STATUS.append("YES")
except:
        INIT_STATUS.append("NO")

try:
        pwm_left_arm = Adafruit_PCA9685.PCA9685(address=0x41)
        pwm_left_arm.set_pwm_freq(60)
        Names['left'] = pwm_left_arm
        INIT_STATUS.append("YES")
except:
        INIT_STATUS.append("NO")

try:
        pwm_right_arm = Adafruit_PCA9685.PCA9685(address=0x42)
        pwm_right_arm.set_pwm_freq(60)
        Names['right'] = pwm_right_arm
        INIT_STATUS.append("YES")
except:
        INIT_STATUS.append("NO")

HOST = ''  # Symbolic name meaning all available interfaces
PORT = 2222 # Arbitrary non-privileged port

ser = ''

LED_COUNT      = 16      # Number of LED pixels.
#LED_PIN        = 22      # GPIO pin connected to the pixels (18 uses PWM!).
LED_PIN        = 10      # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 5       # DMA channel to use for generating signal (try 5)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL    = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53

def colorWipe(strip, color, wait_ms=50):
	"""Wipe color across display a pixel at a time."""
	for i in range(strip.numPixels()):
		strip.setPixelColor(i, color)
	strip.show()


def moove_to(new_pos):
    pwm.set_pwm(1,0,new_pos)
    pwm.set_pwm(0,0,new_pos)

# Intialize the library (must be called once before other functions).
try:
        strip.begin()
        INIT_STATUS.append("YES")
except:
        INIT_STATUS.append("NO")

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('Socket created')

try:
    ser = serial.Serial()
    ser.port = '/dev/ttyACM0'
    ser.baudrate = 115200
    ser.open()
    INIT_STATUS.append("YES")
except:
    INIT_STATUS.append("NO")

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

print("Status of connection: \n PWM (0x40) - {}\n PWM (0x41) - {}\n PWM (0x42) - {}\n LEDs - {} \n Serial - {}\n".format(*INIT_STATUS))


# Function for handling connections. This will be used to create threads
def clientthread(conn):
    # Sending message to connected client
    conn.sendall(b'Welcome to the server. Type something and hit enter\n')  # send only takes string

    # infinite loop so that function do not terminate and thread do not end.
    while True:

        # Receiving from client
        data = conn.recv(1024)
        MyData = (data.decode("utf-8").strip())               

        if (MyData[0] == 'C'):
                s = MyData[1:].split('_')
                print(int(float(s[0])),int(float(s[1])),int(float(s[2])))
        elif (MyData[0] == "S"):
                print(MyData)
                tmp = MyData[1:].split("/")
                tmp2 = tmp[0].split('_')
                if (len(tmp) == 2):
                        print("tmp: {}\ntmp2: {}".format(tmp,tmp2))
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
