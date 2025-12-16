include <Diapasonix.scad>

module neck_shape(_offset = 0){
    $fn=resolution;
    r1 = neck_r1 - _offset;
    r2 = neck_r2 - _offset;
    intersection(){
        translate([-neck_w/2,0,0])
        rotate([0,90,0])
        linear_extrude(neck_w)
        circle(r1);
        
        // Side curvature
        hull(){
            translate([-neck_w/2+r2,r1,0])
            rotate([90,0, 0])
            linear_extrude(neck_d)
            offset(-_offset)
            circle(r2);
            
            translate([neck_w/2-r2*2,r1,-r2])
            rotate([90,0, 0])
            linear_extrude(neck_d)
            offset(-_offset)
            square(r2*2);
        };
        
        // Keep only the bottom half
        translate([-neck_w/2,-neck_w/2,-neck_w])
        cube(neck_w);
    }
}

module battery_holder() {
    difference(){
        // Main shape
        x1 = battery_holder_x1;
        x2 = battery_holder_x2;
        
        translate([battery_holder_pos_x, neck_d / 2, 0])
        rotate([90, 0, -90])
        linear_extrude(battery_holder_extrude_w)
        polygon([
            [0, neck_r1], 
            [x1, neck_r1], 
            [x2, battery_holder_y2], 
            [neck_d - x2, battery_holder_y2], 
            [neck_d - x1, neck_r1], 
            [neck_d, neck_r1], 
            [neck_d, 0], 
            [0, 0]
        ]);
        
        // Battery socket
        translate(center(battery_socket_w + battery_socket_clearance, battery_socket_d + battery_socket_d_offset, 0)) 
        cube([battery_socket_w + battery_socket_wall, battery_socket_d + battery_socket_d_offset, battery_socket_h]);
        
        // Negative terminal socket
        translate([battery_terminal_neg_x, 0, 0]) 
        translate([0, -battery_terminal_neg_y_offset, 0]) 
        cube([battery_terminal_w, battery_terminal_d, battery_socket_h]);
        
        // Positive terminal socket
        translate([battery_terminal_pos_x, 0, 0]) 
        translate(center(battery_terminal_pos_w, battery_terminal_d, 0)) 
        cube([battery_terminal_w, battery_terminal_d, battery_socket_h]);
        
        // Terminal openings
        translate([battery_terminal_opening_x, 0, battery_terminal_opening_z]) 
        rotate([0, 90, 0]) 
        cylinder(h = battery_terminal_opening_h, r = battery_terminal_opening_r);
        
        // Wire conduits
        translate([battery_wire_conduit_x, -battery_wire_conduit_y, battery_wire_conduit_z]) 
        rotate([0, 90, 0]) 
        cylinder(h = battery_wire_conduit_h, r = battery_wire_conduit_r);
        translate([battery_wire_conduit_x, battery_wire_conduit_y, battery_wire_conduit_z]) 
        rotate([0, 90, 0]) 
        cylinder(h = battery_wire_conduit_h, r = battery_wire_conduit_r);
        
        // + symbol
        translate([battery_symbol_pos_x_plus, battery_symbol_y, battery_symbol_z])
        union(){
            cube([battery_symbol_w, battery_symbol_h, battery_symbol_h]);
            translate([battery_symbol_cross_offset, 0, battery_symbol_cross_z_offset])
            cube([battery_symbol_h, battery_symbol_h, battery_symbol_cross_h]);
        }
        
        // - symbol
        translate([battery_symbol_pos_x_minus, battery_symbol_y, battery_symbol_z])
        cube([battery_symbol_w, battery_symbol_h, battery_symbol_h]);
    }
}
module speaker_holder() {
    difference(){
        union(){
            translate([speaker_pos_x1, -neck_d / 2, 0]) 
            cube([speaker_pos_w1, neck_d, speaker_h]);
            translate([speaker_spacing, 0, 0]) 
            translate(center(speaker_pos_w2_center, neck_d, 0)) 
            cube([speaker_pos_w2, neck_d, speaker_h]);
        }
        #translate([speaker_screw_x1, 0, speaker_h - speaker_screw_z_offset]) 
        cylinder(h = speaker_screw_h, r = screw_hole_r);
        #translate([speaker_spacing + speaker_screw_x2_offset, 0, speaker_h - speaker_screw_z_offset]) 
        cylinder(h = speaker_screw_h, r = screw_hole_r);
    }
}

/*
module pico_holder(){
    difference(){
            cube([4,neck_d,8.5]);
            // Screw holes 
            translate([2,neck_d/2-5.68,2])
            cylinder(h=6.3, r=screw_hole_r);
            translate([2,neck_d/2+5.68,2])
            cylinder(h=6.3, r=screw_hole_r);
        }
    difference(){
     // Main shape
    x1 = 0;
    x2 = 16.0;
    y2 = 4.8;
        
    translate([4,0,0])
    rotate([90,0,90])
    linear_extrude(50)
    polygon([
    [0, neck_r1], [x1,neck_r1], [x2, y2], [neck_d-x2, y2], [neck_d-x1, neck_r1],[neck_d, neck_r1], [neck_d,0], [0, 0],
    ]);
    }
    
    //translate([4,neck_d/2-21/2,5.5])
    //#cube([21,21,1]);
}*/

module shell_inner_supports() {
    intersection(){
        translate([neck_w/2,neck_d/2,0])
            neck_shape(_offset=1);
        
        union(){
            wedge_h = neck_r1*.57;
        // Pico support
        //translate([neck_w-50-wall_d,0,-neck_r1])
        //pico_holder();
            
        // Inset reinforcement
        translate([neck_w-wall_d,neck_r1-(tray_w+tray_m*2)/2,-neck_r1])
        rotate([0,-90,0])
        cube([neck_r1,tray_w+tray_m*2,wall_d]);
        
        // Inner holders
        translate([neck_w - shell_battery_holder_x, neck_d / 2, -neck_r1])
        battery_holder();
        
        translate([shell_speaker_offset_x, neck_d / 2, -neck_r1])
        speaker_holder();
        };
    };
}

module shell(){
    $fn=resolution;
        translate([neck_w/2,neck_d/2,0])
        difference(){
            neck_shape();
            neck_shape(_offset=wall_d);
        };
}

/*
shell();
shell_inner_supports();
//*/