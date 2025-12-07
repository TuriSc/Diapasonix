include <Diapasonix.scad>
include <shell.scad>

module body(){
    difference(){
        union(){
            shell();
            shell_inner_supports();
        }
        // BOOTSEL opening
        translate([neck_w - bootsel_x, neck_d / 2 - bootsel_y, bootsel_z])
        rotate([0, 0, -90])
        cylinder(h = bootsel_h, r = bootsel_r);
        
        // Tray opening
        margin = pcb_clearance;
        translate([neck_w - tray_inner_w - margin, neck_r1 - margin, 0])
        rotate([0, 90, 0])
        linear_extrude(tray_inner_w + margin)
        intersection(){
            circle(neck_r1 - tray_m + margin, $fn = resolution);
            
            translate([neck_r1 - tray_m - tray_h, -tray_w / 2, 0])
            square([neck_r1 * 2, tray_w + margin * 2]);
        }
        
        // Eyelet insets
        translate([neck_w - wall_d * 2, neck_r1 - margin, 0])
        translate([wall_d * 1.5 - pcb_clearance, tray_w / 2 + tray_eyelet_y_offset, tray_eyelet_z])
        rotate([90, 0, 90])
        linear_extrude(wall_d / 2 + pcb_clearance)
        hull(){
            circle(eyelet_r + pcb_clearance, $fn = resolution);
            translate([-eyelet_r - pcb_clearance, -eyelet_r - pcb_clearance, 0])
            square([eyelet_square_w, eyelet_square_d + pcb_clearance * 2]);
        }
        
        translate([neck_w - wall_d * 2, neck_r1 - margin, 0])
        translate([wall_d * 1.5 - pcb_clearance, -tray_w / 2 - tray_eyelet_y_offset, tray_eyelet_z])
        rotate([-90, 0, -90])
        linear_extrude(wall_d / 2 + pcb_clearance)
        hull(){
            circle(eyelet_r + pcb_clearance, $fn = resolution);
            translate([-eyelet_r - pcb_clearance, -eyelet_r - pcb_clearance, 0])
            square([eyelet_square_w, eyelet_square_d + pcb_clearance * 2]);
        }
        
        // Eyelet screw holes
        translate([neck_w - wall_d * 2, neck_r1 - margin, 0])
        translate([0, tray_w / 2 + tray_eyelet_y_offset, tray_eyelet_z])
        rotate([90, 0, 90])
        translate([0, 0, eyelet_countersunk_z])
        cylinder(h = screw_hole_depth, r = screw_r, $fn = 64);
        
        translate([neck_w - wall_d * 2, neck_r1 - margin, 0])
        translate([0, -tray_w / 2 - tray_eyelet_y_offset, tray_eyelet_z])
        rotate([-90, 0, -90])
        translate([0, 0, eyelet_countersunk_z])
        cylinder(h = screw_hole_depth, r = screw_r, $fn = 64);
        
        // Speaker holes
        translate([neck_w - shell_battery_offset_x + speaker_spacing, neck_d / 2, -neck_r1])
        for (a = [0:7]) {
            rotate(45 * a)
            translate([11, 0, 5])
            cylinder(h = 10, r = 1.4, center = true);
            if(a % 2 == 0){
                rotate(45 * a)
                translate([4.5, 0, 5])
                cylinder(h = 10, r = 1.4, center = true);
            }
        }
        
        // Bottom screw
        translate([neck_w - wall_d * 2, neck_r1, 0])
        rotate([-180, 0, 0])
        translate([wall_d + tray_bottom_screw_x, 0, neck_r1 - tray_bottom_screw_z_offset])
        countersunk();
    }
    
    // Top face
    difference(){
        union(){
            translate([0, 0, -wall_d])
            cube([neck_w, neck_d, wall_d]);
            
            // Navigation switch support
            translate([neck_w - oled_pos_x + nav_switch_pos_x_offset + nav_switch_pos_y_offset, neck_d / 2 - nav_switch_pos_y_base - wall_d, -nav_switch_pos_h - wall_d])
            cube([nav_switch_cross_w, nav_switch_cross_d - wall_d * 2, wall_d + nav_switch_pos_h]);
            
            // OLED underside supports
            translate([neck_w - oled_pos_x - 5.5, neck_d / 2 - oled_support_top_offset, -wall_d - 2])
            linear_extrude(wall_d)
            polygon([[0, 0], [9, 0], [0, 9]]);
            
            translate([neck_w - oled_pos_x - 5.5, neck_d / 2 + oled_support_bottom_offset, -wall_d - 2])
            linear_extrude(wall_d)
            polygon([[0, 0], [0, 9], [9, 9]]);
            
            translate([neck_w - oled_ribbon_x + oled_ribbon_w, neck_d / 2 - oled_support_top_offset, -wall_d - 2])
            linear_extrude(wall_d)
            polygon([[0, 0], [9, 0], [9, 9]]);
            
            translate([neck_w - oled_ribbon_x + oled_ribbon_w, neck_d / 2 + oled_support_bottom_offset, -wall_d - 2])
            linear_extrude(wall_d)
            polygon([[9, 0], [0, 9], [9, 9]]);
        }
        
        // PCB opening
        translate([wall_d + 2, wall_d + pcb_pos_y_offset + 6.5, -wall_d])
        cube([pcb_w - 4, pcb_d - 13, wall_d]);
    
        // Screw holes, PCB
        translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, -wall_d])
        cylinder(r = screw_r, h = wall_d, $fn = 64);
        
        translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, -wall_d])
        cylinder(r = screw_r, h = wall_d, $fn = 64);
        
        translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, -wall_d])
        cylinder(r = screw_r, h = wall_d, $fn = 64);
        
        translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, -wall_d])
        cylinder(r = screw_r, h = wall_d, $fn = 64);
        
        // Screw holes, UI
        translate([neck_w - ui_screw_x_offset, wall_d + pcb_screw_offset_y + screw_pos_y_offset, -wall_d])
        cylinder(r = screw_r, h = wall_d, $fn = 64);
        
        // Screw holes, OLED
        translate([neck_w - oled_screw_x1, neck_d / 2 + oled_screw_y1, -7.2])
        cylinder(r = screw_r, h = 5, $fn = 64);
        translate([neck_w - oled_screw_x2, neck_d / 2 + oled_screw_y1, -7.2])
        cylinder(r = screw_r, h = 5, $fn = 64);
        translate([neck_w - oled_screw_x2, neck_d / 2 - oled_screw_y2, -7.2])
        cylinder(r = screw_r, h = 5, $fn = 64);
        translate([neck_w - oled_screw_x1, neck_d / 2 - oled_screw_y2, -7.2])
        cylinder(r = screw_r, h = 5, $fn = 64);
        
        // OLED opening
        translate([neck_w - oled_pos_x - 5.1, neck_d / 2 - oled_opening_top_offset, -2])
        cube([oled_opening_w, oled_opening_d, 3]);
        
        // Navigation switch opening
        translate([neck_w - oled_pos_x + nav_switch_pos_x_offset + 0.2, neck_d / 2 - nav_switch_pos_y_base, -3.8])
        cube([10.4, 10.8, 10.4]);
    }

    // Standoffs, PCB
    translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, -wall_d - 2])
    standoff(2);
    
    translate([wall_d + pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, -wall_d - 2])
    standoff(2);
    
    translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y + screw_pos_y_offset, -wall_d - 2])
    standoff(2);
    
    translate([wall_d + pcb_w - pcb_screw_offset_x, wall_d + pcb_screw_offset_y_bottom + screw_pos_y_offset, -wall_d - 2])
    standoff(2);
    
    // Standoffs, UI
    translate([neck_w - ui_screw_x_offset, wall_d + pcb_screw_offset_y + screw_pos_y_offset, -wall_d - 2])
    standoff(2);
}

if(is_undef(is_root)) {
    body();
}