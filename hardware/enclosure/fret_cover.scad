include <Diapasonix.scad>

fret_cover_h = 2;
fret_cover_choke_h = 0.2;
fret_cover_delta = 1.5;

module fret_cover(){
    
    difference(){
            hull(){
    hull(){
    translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, 0])
    cylinder(h=fret_cover_h, r=cap_header_cover_r);
    
    translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, 0])
    cylinder(h=fret_cover_h, r=cap_header_cover_r);
    }
    hull(){
    translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, 0])
    cylinder(h=fret_cover_h, r=cap_header_cover_r);
    
    translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, 0])
    cylinder(h=fret_cover_h, r=cap_header_cover_r);
    }
}
        
        
        // Fret cutouts, larger
        translate([top_opening_x + fret_cutout_x_offset, pod_d + ridge_d / 2 + r_clearance, 0])
        for (s = [0:n_strings-1]) {
            for (f = [0:n_pods_per_string-1]) {
                translate([f * fret_spacing, s * pod_d, 0])
                linear_extrude(fret_cover_h - fret_cover_choke_h)
                hull(){
                    translate([fret_r, 0, 0])
                    circle(fret_r + r_clearance);
                    translate([fret_w - fret_r, 0, 0])
                    circle(fret_r + r_clearance);
                }
            }
        }
        
                // Fret cutouts, smaller
        #translate([top_opening_x + fret_cutout_x_offset, pod_d + ridge_d / 2 + r_clearance,0])
        for (s = [0:n_strings-1]) {
            for (f = [0:n_pods_per_string-1]) {
                translate([f * fret_spacing, s * pod_d, 0])
                linear_extrude(fret_cover_h)
                hull(){
                    translate([fret_r ,0, 0])
                    circle(fret_r + r_clearance - fret_cover_delta);
                    translate([fret_w - fret_r, 0, 0])
                    circle(fret_r + r_clearance - fret_cover_delta);
                }
            }
        }
    
        // Screw holes, PCB
        translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, 0])
        countersunk();
    
        translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, 0])
        countersunk();
    
        translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, 0])
        countersunk();
    
        translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, 0])
        countersunk();


    }
}

if(is_undef(is_root)) {
    fret_cover();
}