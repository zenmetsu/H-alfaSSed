# H-αlfaSSed
Hydrogen-αlfa Sensor Stream editor

Imaging sensors based upon the IMX178 and IMX183 from Sony have a particularly hard time 
with long wavelength monochromatic images. The resulting images will often contain a 
grid-like artifact where every other column has alternating over-biased and under-biased
pixels.  This utility will read a .SER stream, analyze each frame to determine the correct
bias, and then write a new .SER with the corrected data.
