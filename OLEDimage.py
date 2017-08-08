# OLEDimage.py

# Meant for use with the Raspberry Pi and an Adafruit monochrome OLED display!

# This program takes any image (recommended: landscape) and converts it into a black and white image which is then 
# displayed on one of Adafruit's monochrome OLED displays. 

# To run the code simply change directory to where it is saved and then type: sudo python OLEDimage.py <image name>
# For example: sudo python OLEDimage.py penguins900x600.jpg 
# The image penguins900x600.jpg is included in this repo as an example! Download any image that you want and simply change the
# image name!

# This program was created by The Raspberry Pi Guy

# Imports the necessary software - including PIL, an image processing library
import gaugette.ssd1306
import time
import sys
from PIL import Image

# Sets up our pins and creates variables for the size of the display. If using other size display you can easily change them.

# Define which GPIO pins the reset (RST) and DC signals on the OLED display are connected to on the
# Raspberry Pi. The defined pin numbers must use the WiringPi pin numbering scheme.
RESET_PIN = 15 # WiringPi pin 15 is GPIO14.
DC_PIN = 16 # WiringPi pin 16 is GPIO15.
width = 128
height = 128

led = gaugette.ssd1306.SSD1306(reset_pin=RESET_PIN, dc_pin=DC_PIN)
led.begin()
led.clear_display()

def draw_image(width,height,image_name):
        global led
        for x in range(width):
                for y in range(height):
                        led.draw_pixel(x,y,bool(int(image_name.getpixel((x,y)))))



def create_animation(filename_base, count, sleep_time = 1):
        global led
        for i in  range(0,count):
                name = '%05d' % i
                name = filename_base + name
                image = Image.open(name)
                draw_image(128,128,image)
                led.display()
                time.sleep(sleep_time)
                
        
