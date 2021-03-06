// Persistence Of Vision raytracer sample file.
// Quilted pattern example
//
// -w320 -h240
// -w800 -h600 +a0.3

#version 3.6;

global_settings {
  assumed_gamma 2.2
}

#include "colors.inc"

#declare T1=
 texture{
   pigment{
     quilted color_map{[0.0 Black][1.0 White]}
     control0 1 control1 1
     scale 0.3
   }
 }

#declare T2=
 texture{
   pigment{White}
   normal{
     quilted 0.4
     control0 1 control1 1
     scale 0.3
   }
   finish{phong 0.8 phong_size 200}
 }

#include "pignorm.inc"
