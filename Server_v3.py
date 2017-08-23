try:
        import socket
        import sys
        from _thread import *
        from neopixel import *
        import time
        import serial
        import Adafruit_PCA9685
except:
        pass

INIT_STATUS = [] ## Статус подключения модулей

Names = {} ## PCA модули по именам


## Следующий блок кода отвечает за подключение PCA.


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


## Задаём хост, и порт. Пустые кавычки равносильны localhost
HOST = ''
PORT = 8888

## Задаём переменную Serial-порта. Для упарвления глазами
ser = '' 

try:
## Задаём переменные, необходмые для управления подсветкой.
        LED_COUNT      = 16      # Number of LED pixels.
        #LED_PIN        = 22      # GPIO pin connected to the pixels (18 uses PWM!).
        LED_PIN        = 10      # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
        LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
        LED_DMA        = 5       # DMA channel to use for generating signal (try 5)
        LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
        LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
        LED_CHANNEL    = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53
        LED_STRIP      = ws.WS2811_STRIP_GRB   # Strip type and colour ordering
except:
        pass


def colorWipe(strip, color, wait_ms=50):
	''' Функция одновременно включает все светодиоды заданного цвета '''
	for i in range(strip.numPixels()):
		strip.setPixelColor(i, color)
	strip.show()


def moove_to(new_pos):
    pwm.set_pwm(1,0,new_pos)
    pwm.set_pwm(0,0,new_pos)

## Создаём и проверяем подключение светодиодов
try:
        strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL, LED_STRIP)
except:
        pass
# Intialize the library (must be called once before other functions).
try:
        strip.begin()
        INIT_STATUS.append("YES")
except:
        INIT_STATUS.append("NO")

## Создаём сокет
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('Socket created')


## Пытаемся подключиться к Teensy, по Serial-порту
try:
    ser = serial.Serial()
    ser.port = '/dev/ttyACM0'
    ser.baudrate = 115200
    ser.open()
    INIT_STATUS.append("YES")
except:
    INIT_STATUS.append("NO")


## Запускаем обработчик сокетов
try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
    sys.exit()


print('Socket bind complete')

# Start listening on socket
s.listen(10)
print ('Socket now listening')

## Выводим статус подключения
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

        if (MyData[0] == "E"): ##Движение глаз по горизонтали
            s = MyData[1:]+"g"
            ser.write(s.encode('utf-8')) 
        elif (MyData[0] == "V"): ##Движение глаз по вертикали
            s = MyData[1:]+"v"
            ser.write(s.encode('utf-8'))
        elif (MyData[0] == "n"):
            ser.write('n'.encode('utf-8')) ##Переключение между автоматическим движением и ручным управлением глаз
        elif (MyData[0] == "h"):
            ser.write('h'.encode('utf-8')) ##Переключение между глазами и картинокой с сердцем
        elif (MyData[0] == "b"):
            ser.write('b'.encode('utf-8'))
        elif (MyData[0] == "l"):
            ser.write('l'.encode('utf-8'))
        elif (MyData[0] == "r"):
            ser.write('r'.encode('utf-8'))
        elif (MyData[0] == "S"): ## Управление сервоприводами
                tmp = MyData[1:].split("/")
                tmp2 = tmp[0].split('_')
                if (len(tmp) == 2):
                        print("tmp: {}\ntmp2: {}".format(tmp,tmp2))
                        try:
                                Names[tmp2[0]].set_pwm(int(tmp2[1]),0,int(float(tmp[1])))
                        except Exception as e:
                                print(e)
        elif (MyData[0] == 'C'): ## Управление цветом.
                s = MyData[1:].split('_')
                colorWipe(strip,Color(int(float(s[0])),int(float(s[1])),int(float(s[2]))))
                
                
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
