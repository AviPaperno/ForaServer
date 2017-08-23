#!/usr/bin/python

# Image converter for 'Uncanny Eyes' project.  Generates tables for
# eyeData.h file.  Requires Python Imaging Library.  Expects four image
# files: sclera, iris, upper lid map and lower lid map (defaults will be
# used if not specified).  Also generates polar coordinate map for iris
# rendering (pass diameter -- must be an even value -- as 5th argument).
# Output is to stdout; should be redirected to file for use.

# This is kinda some horrible copy-and-paste code right now for each of
# the four images...could be improved, but basically does the thing.

import sys
import math
from PIL import Image


columns = 8 # Number of columns in formatted output

# Write hex digit (with some formatting for C array) to stdout
def outputHex(n, digits):
	global columns, column, counter, limit
	column += 1                        # Increment column number
	if column >= columns:              # If max column exceeded...
		sys.stdout.write("\n  ")   # end current line, start new one
		column = 0                 # Reset column number
	sys.stdout.write("{0:#0{1}X}".format(n, digits + 2))
        counter += 1
        if counter < limit:                # If NOT last element in list...
		sys.stdout.write(",")      # add column between elements
		if column < (columns - 1): # and space if not last column
			sys.stdout.write(" ")
	else:
		f.write(" };");              # Cap off table


# OPEN AND VALIDATE SCLERA IMAGE FILE --------------------------------------


im     = Image.open('img.jpg')
im     = im.convert("RGB")
pixels = im.load()

# GENERATE SCLERA ARRAY ----------------------------------------------------

# Initialize outputHex() global counters:
counter = 0                       # Index of next element to generate
column  = columns                 # Current column number in output
limit   = im.size[0] * im.size[1] # Total # of elements in generated list

print ("#define IMG_WIDTH  " + str(im.size[0]))
print ("#define IMG_HEIGHT " + str(im.size[1]))
print()

sys.stdout.write("const uint16_t heart[IMG_WIDTH][IMG_HEIGHT] = {")

# Convert 24-bit image to 16 bits:
for y in range(im.size[1]):
	for x in range(im.size[0]):
		p = pixels[x, y] # Pixel data (tuple)
		outputHex(((p[0] & 0b11111000) << 8) | # Convert 24-bit RGB
		          ((p[1] & 0b11111100) << 3) | # to 16-bit value w/
		          ( p[2] >> 3), 4)             # 5/6/5-bit packing
