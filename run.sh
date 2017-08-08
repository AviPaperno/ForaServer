sudo apt-get install python-smbus
sudo apt-get install i2c-tools
sudo apt-get install build-essential python-dev git scons swig


sudo apt-get install git build-essential python-dev
cd ~
git clone https://github.com/adafruit/Adafruit_Python_PCA9685.git
cd Adafruit_Python_PCA9685
sudo python setup.py install
sudo python3 setup.py install


git clone https://github.com/jgarff/rpi_ws281x.git
cd rpi_ws281x
scons

cd python
sudo python3 setup.py install
