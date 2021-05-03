# H-αlfaSSed
Hydrogen-αlfa Ser Stream editor

Monochrome imaging sensors based upon the IMX178 and IMX183 from Sony have a particularly hard time 
with long wavelength monochromatic images. The resulting images will often contain a 
grid-like artifact where every other column has alternating over-biased and under-biased
pixels.  This utility will read a .SER stream, analyze each frame to determine the correct
bias, and then write a new .SER with the corrected data.

## Description of the problem:


Despite being monochrome sensors, the monochrome variants of the IMX178 and IMX183 appear to have
pixel bias which resembles a typical bayer mask.  

Here is an example of an image exhibiting the problem:
![alt text](https://raw.githubusercontent.com/zenmetsu/H-alfaSSed/master/source.png)


To explain this better, we must de-bayer the image
with the following pixel format:

RED GRN
GRN BLU

The de-bayer process is simple, one need only apply a modulo function to the row and column number.  
The red channel is represented by pixels where the row and column are even. The blue channel is
represented by pixels where the row and column are odd.  The green channels are the remaining two, where
the one of the row or column values are odd.

If one de-bayers the image into four arrays, the problem becomes evident as the red channel will have a higher
mean value than the two green channels, and the blue channel will have a slightly lower mean.  The two green
channels will be nearly identical.

The problem, besides the obvious error in the sensor data, is that imaging stacking software will often lock 
onto this grid pattern as it is a relatively strong signal.  As a consequence, the resulting image will
appear to have 1/4 the sensor resolution, with details lost due to the image stacking error.

## Solution:

This utility works by debayering each frame of the .ser file, normalizing the red and blue channels to match
the observed green channel data, prior to writing a corrected output file.  

Here is the frame of data from before, with the necessary corrections applied:
![alt text](https://raw.githubusercontent.com/zenmetsu/H-alfaSSed/master/output.png)

## CAVEATS

First of all, garbage in, garbage out.  The data from these sensors will be garbage, and there will be some 
loss of dynamic range.  This cannot be helped.  As a consequence, it is best to try to maximize the dynamic 
range at time of capture.

Second, if any pixels are overexposed, it will introduce error into the red and blue channel correction values.
This problem may be minimal, or it might produce unacceptable results.

Third, even after correction, there is still some observed error.  Usually this is inconsequential as the signal
from the grid error is usually attenuated sufficiently to allow image stacking without loss of detail.  As the
images are stacked, the grid error is usually removed through the stacking process.
