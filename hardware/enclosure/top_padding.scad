include <Diapasonix.scad>

module top_padding(){
    // Top face
    difference(){
        cube([neck_w, neck_d, wall_d + pcb_clearance]);
        
        // PCB
        #translate([wall_d - pcb_clearance, wall_d + pcb_pos_y_offset - pcb_clearance, 0])
        cube([pcb_w + pcb_clearance_2x, pcb_d + pcb_clearance_2x, wall_d + pcb_clearance]);
    
        // Screw holes, UI
        translate([neck_w - ui_screw_x_offset, wall_d + pcb_screw_offset_y + screw_pos_y_offset, countersunk_offset])
        countersunk();
    
        // Screw holes, OLED
        translate([neck_w - oled_screw_x1, neck_d / 2 + oled_screw_y1, countersunk_offset])
        countersunk();
        translate([neck_w - oled_screw_x2, neck_d / 2 + oled_screw_y1, countersunk_offset])
        countersunk();
        translate([neck_w - oled_screw_x2, neck_d / 2 - oled_screw_y2, countersunk_offset])
        countersunk();
        translate([neck_w - oled_screw_x1, neck_d / 2 - oled_screw_y2, countersunk_offset])
        countersunk();
    
        // OLED opening
        translate([neck_w - oled_pos_x - oled_clearance, neck_d / 2 - oled_pos_y_offset - oled_clearance, 0])
        cube([oled_opening_w, oled_opening_d, oled_opening_z]);
        
        // OLED ribbon opening
        translate([neck_w - oled_ribbon_x_top, neck_d / 2 + oled_ribbon_y_offset, oled_ribbon_z_offset + wall_d - oled_clearance])
        cube([oled_ribbon_w, oled_ribbon_d, oled_ribbon_z]);
        
        // OLED header opening
        translate([neck_w - oled_header_x, neck_d / 2 + oled_header_y_offset, 0])
        cube([header_w, header_oled_l, wall_d + pcb_clearance + oled_header_z_offset]);
        
        // Navigation switch opening
        translate([neck_w - oled_pos_x + nav_switch_pos_x_offset + nav_switch_pos_y_offset_top, neck_d / 2 - nav_switch_pos_y_base + nav_switch_pos_y_offset_top, nav_switch_pos_z])
        cylinder(r = nav_switch_r_opening, h = nav_switch_pos_h, center = true);
    }
}

if(is_undef(is_root)) {
    top_padding();
}