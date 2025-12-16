include <Diapasonix.scad>
include <shell.scad>

$fn=resolution;

module chassis(){
    translate([-wall_d, 0, 0])
    difference(){
        // Main shape
        hull(){
            translate([chassis_x, chassis_y1, chassis_z1])
            linear_extrude(chassis_h1)
            square([chassis_w, chassis_d1]);
        
            translate([chassis_x, chassis_y2, chassis_z2])
            linear_extrude(chassis_h2)
            square([chassis_w, chassis_d2]);
        }
        
        // Cuts
        hull(){
            translate([chassis_cut1_x, chassis_cut1_y, chassis_cut1_z1])
            linear_extrude(chassis_h2)
            square([chassis_cut1_w1, chassis_cut1_d]);
        
            translate([chassis_cut1_x - chassis_cut1_x_offset, chassis_cut1_y, chassis_cut1_z2])
            linear_extrude(chassis_h2)
            square([chassis_cut1_w2, chassis_cut1_d]);
        }
        
        hull(){
            translate([chassis_cut2_x, chassis_cut1_y, chassis_cut1_z1])
            linear_extrude(chassis_h2)
            square([chassis_cut2_w1, chassis_cut1_d]);
        
            translate([chassis_cut2_x2, chassis_cut1_y, chassis_cut1_z2])
            linear_extrude(chassis_h2)
            square([chassis_cut2_w2, chassis_cut1_d]);
        }
        
        translate([chassis_cut3_x, chassis_cut3_y, chassis_cut3_z])
        cube([chassis_cut3_w, chassis_cut3_d, chassis_cut3_h]);
        
        // Compensate for rendering error
        translate([render_compensation, 0, 0]){
            // TP4056 PCB slot
            translate([tp4056_pcb_x, tp4056_pcb_y, tp4056_pcb_z])
            cube([tp4056_pcb_w, tp4056_pcb_d, tp4056_pcb_h]);
            
            // Solder clearances
            translate([tp4056_solder_x + render_compensation, tp4056_solder_y, tp4056_solder_z])
            cube([tp4056_solder_w, tp4056_solder_d, tp4056_pcb_h]);
        }
       
        // Screw holes, Pico
        translate([pico_screw_x, -pico_screw_y, chassis_z2])
        cylinder(h = pico_screw_h, r = screw_hole_r);
        translate([pico_screw_x, pico_screw_y, chassis_z2])
        cylinder(h = pico_screw_h, r = screw_hole_r);
        
        // Screw holes, amp
        translate([amp_screw_x, -amp_screw_y, amp_screw_z])
        cylinder(h = amp_screw_h, r = screw_hole_r);
        translate([amp_screw_x, amp_screw_y, amp_screw_z])
        cylinder(h = amp_screw_h, r = screw_hole_r);
        
        // TP4056 stop
        translate([tp4056_stop_x, 0, tp4056_stop_z])
        cylinder(h = tp4056_stop_h, r = screw_hole_r);
    }
}

module eyelet() {
    difference(){
        linear_extrude(wall_d / 2)
        hull(){
            circle(eyelet_r, $fn = resolution);
            translate([-eyelet_square_offset, -eyelet_square_offset, 0])
            square([eyelet_square_w, eyelet_square_d]);
        }
        translate([0, 0, eyelet_countersunk_z])
        countersunk();
    }
}

module tray_base(_offset=tray_m, width=tray_w, depth=wall_d){
    linear_extrude(depth)
    intersection(){
        circle(neck_r1-_offset,$fn=resolution*2);
        translate([neck_r1-_offset-tray_h,-width/2,0])
        square([neck_r1*2, width]);
    }
}

module tray(){
    difference(){
        union(){
            // Base
            rotate([0, 90, 0]) {
                tray_base();
            
                translate([0, 0, -tray_inner_w])
                difference(){
                    tray_base(depth = tray_inner_w);
                    tray_base(_offset = tray_m + 1, width = tray_w - 2, depth = tray_inner_w);
                }
            }
        
            // Chassis
            translate([tray_chassis_offset_x, 0, tray_chassis_offset_z])
            chassis();
        
            // Top screw eyelets
            translate([wall_d / 2, tray_w / 2 + tray_eyelet_y_offset, tray_eyelet_z])
            rotate([90, 0, 90])
            eyelet();
        
            translate([wall_d / 2, -tray_w / 2 - tray_eyelet_y_offset, tray_eyelet_z])
            rotate([-90, 0, -90])
            eyelet();
        }
        
        union(){
            // Pico USB opening        
            translate([wall_d - pico_usb_x_offset, 0, pico_usb_z])
            cube([pico_usb_w, pico_usb_d, pico_usb_h], center = true);
        
            // Pico PCB
            translate([0, 0, pico_pcb_z])
            cube([wall_d * 2 - pico_pcb_w_clearance, pico_pcb_d, pico_pcb_h], center = true);
        
            // TP4056 PCB
            translate([0, 0, tp4056_pcb_tray_z])
            cube([wall_d * 2 - pico_pcb_w_clearance, tp4056_pcb_d, pico_pcb_h], center = true);
        
            // TP4056 socket and LED opening
            translate([0, 0, tp4056_socket_z])
            rotate([90, 0, 90])
            linear_extrude(tp4056_socket_extrude_h) {
                hull(){
                    translate([-tp4056_socket_hole_x, 0, 0]) 
                    circle(tp4056_socket_hole_r, $fn = 64);
                    translate([tp4056_socket_hole_x, 0, 0]) 
                    circle(tp4056_socket_hole_r, $fn = 64);
                }
                translate([-tp4056_led_x, 0, 0]) 
                circle(tp4056_led_r, $fn = 64); // LED hole
            }
        }
    
        // BOOTSEL opening
        *translate([bootsel_x, bootsel_y, bootsel_z])
        rotate([0, 0, -90])
        cylinder(h = bootsel_h, r = bootsel_r);
    }
}

if(is_undef(is_root)) {
    color("gold") tray();
}