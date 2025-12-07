include <Diapasonix.scad>

module fret(mag_holes=true) {
    difference(){
        translate([0, 0, -fret_r * 2 + fret_h])
        union() {
            // Dome
            difference(){
                hull(){
                    translate([fret_r, 0, fret_r])
                    sphere(fret_r);
                    translate([fret_w - fret_r, 0, fret_r])
                    sphere(fret_r);
                }
                translate([0, -fret_r, 0])
                cube([fret_w, fret_d, fret_r]);
            }
            
            // Walls
            translate([0, 0, fret_r])
            rotate([180, 0, 0]) 
            linear_extrude(fret_h - fret_r)
            hull(){
                translate([fret_r, 0, 0])
                circle(fret_r);
                translate([fret_w - fret_r, 0, 0])
                circle(fret_r);
            }
        }
    }
}

if(is_undef(is_root)) {
    fret();
}