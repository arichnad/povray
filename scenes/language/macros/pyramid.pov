// Persistence Of Vision Ray Tracer POV-Ray sample Scene
// by Chris Young
//    Based on a POV-Ray 3.0 file by
//    Sven Hilscher * 3D-Max usergroup germany
//    email: sven@rufus.central.de
// PYRAMID.POV demonstrates basic use of macros and local
// identifiers in recursive calls.  Creates a fractal
// pyramid from spheres.
//
// Note some spheres are redundant.  See PYRAMID2.POV
// for a version which eliminates duplicate spheres.
//
// -w320 -h240
// -w800 -h600 +a0.3

// Define the macro.  Parameters are:
//   X:  position of sphere
//   Y:  position of sphere
//   Z:  position of sphere
//   R:  radius of sphere
//   L:  level of recursion

#version 3.6;
global_settings {assumed_gamma 1.0}

#macro Pyramid(X,Y,Z,R,L)

  sphere { <X,Y,Z>,R}

  #if (L > 0)
    #local New_L = L - 1;
    #local New_R = R / 2;
    #local Pos   = New_R * 3;

    Pyramid(X+Pos,Y,Z,New_R,New_L)
    Pyramid(X-Pos,Y,Z,New_R,New_L)
    Pyramid(X,Y+Pos,Z,New_R,New_L)
    Pyramid(X,Y-Pos,Z,New_R,New_L)
    Pyramid(X,Y,Z+Pos,New_R,New_L)
    Pyramid(X,Y,Z-Pos,New_R,New_L)
  #end
#end

union {
  Pyramid(0,0,0,4,6)
  pigment { rgb <0,1,1> }
}

light_source { <20,200,100>, rgb 1 }

background { color rgb <.4, .3, .2> }

camera { location <5,17,19>
         look_at  <0,0,0>
}
