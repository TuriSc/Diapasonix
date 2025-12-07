include <Diapasonix.scad>

module navigation_switch_cap(){
    difference(){
        
        translate([-nav_switch_side/2, -nav_switch_side/2, 0]) 
        cube([nav_switch_side, nav_switch_side, nav_switch_height]);
    
        translate([-nav_switch_cutout_size/2, -nav_switch_cutout_size/2, 0])
        cube([nav_switch_cutout_size, nav_switch_cutout_size, nav_switch_cutout_depth]);
    }
}

if(is_undef(is_root)) {
    translate([0,0,nav_switch_height])
    rotate([180,0,0])
    navigation_switch_cap();
}