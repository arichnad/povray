// Persistence Of Vision raytracer sample file.
// File by Dan Farmer
// Reflection demonstration

#version 3.6;

global_settings {
  assumed_gamma 2.2
  }

#include "colors.inc"
#include "stdcam.inc"

#declare R = clock;

#declare Font="cyrvetic.ttf"
text{ ttf Font
    concat("reflection=",str(R,1,1)),0.1,0
    scale <1.25, 1.25, 4>
    translate <-3.9, 0, 0>
    pigment { rgb <1, 0.5, 0.2> }
    finish { reflection R }
}

sphere { <-1.5, 0.4, -2.5>, 0.4
    pigment { LimeGreen }
    finish { reflection R }
}
