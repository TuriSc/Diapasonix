// To be cleaned up, as I tried different construction
// methods and left a trail of unused variables.

resolution = 128; // Adjust as needed
$fn=resolution;

n_strings = 4;
n_pods_per_string = 6;

pod_w = 25;
pod_d = 10;
pod_h = 2.4;

fret_w = 20;
fret_d = 5;
fret_h = 4; // min is fret_r
fret_r = fret_d/2;
fret_skirt_w = 0.6;
fret_skirt_h = 0.6;

silicone_h = 1;

magnet_r = 1 + 0.25; // Including clearance
magnet_h = 1 + 0.4; // Including clearance

ridge_d = 2.8;
ridge_h = 3;

ui_w = 58;
capo_offset_w = 7;

wall_d = 2.0;

standoff_r = 2.6;
screw_r = 1.2; // 1 is too small
so_clr = 0.4;

neck_w = pod_w*n_pods_per_string+ui_w;
neck_d = pod_d*n_strings+ridge_d*n_strings+standoff_r;

neck_r1=neck_d/2;
neck_r2=36;
tongue = 1.2;
tongue_h = 4.0;
r_clearance = .4;

tray_w = 25;
tray_h = 20;
tray_m = 2;

neck_top_h = pod_h+ridge_h+fret_r;

// Clearances and tolerances
pcb_clearance = 0.4;
pcb_clearance_2x = pcb_clearance * 2;
header_clearance = 0.4;
switch_clearance = 0.8;
switch_clearance_small = 0.4;
groove_depth = 1.6;
countersunk_depth = 0.4;
countersunk_offset = -0.8;
render_compensation = 0.01;
countersunk_height = 3.2;
screw_hole_depth = 8;

// PCB dimensions
pcb_w = 159.790;
pcb_d = 47.675;
pcb_h = 1;
pcb_screw_offset_x = 4.730;
pcb_screw_offset_y = 3.465;
pcb_screw_offset_y_bottom = 47.625 - 3.465;

// Header dimensions
header_w = 2.75;
header_l = 34;
header_h = 3;
header_oled_l = 12;
header_pos_x = 3.38;
header_pos_y = 4.92;
header_pos_y_offset = 3;
header_pos_x2_offset = 3.48; // Offset for second header from PCB edge
pcb_edge_offset = 1; // Offset from PCB edge
pcb_pos_y_offset = 1; // Y offset for PCB position
screw_pos_y_offset = 1; // Y offset for screw positions

// Capacitive headers cover
cap_header_cover_r = 3.6;
cap_header_cover_dist = 42.66;
cap_header_cover_groove_l = 30.70;
cap_header_cover_screw_y1 = 3.465 + 1;
cap_header_cover_screw_y2_offset = 2.5;
cap_header_cover_top_z = 2;
cap_header_cover_top_h = 2;
cap_header_cover_bottom_z = -10;
cap_header_cover_bottom_h = 10;

// Top opening
top_opening_x = 12.4;
top_opening_w = 137.8;
top_opening_d = 40;
top_opening_y_offset = 4.0;
fret_cutout_x_offset = 2;
fret_cutout_z = 1.4;
fret_spacing = 22.88;

// OLED dimensions
oled_w = 17;
oled_d = 25;
oled_clearance = 0.2;
oled_opening_w = oled_w + oled_clearance * 2;
oled_opening_d = oled_d + oled_clearance * 2;
oled_opening_z = 3;
oled_pos_x = 28.5;
oled_pos_y_offset = 2.7;
oled_support_top_offset = 5.2;
oled_support_bottom_offset = 14.2;
oled_opening_top_offset = 4.6;
oled_screw_x1 = 31.1;
oled_screw_x2 = 8.9;
oled_screw_y1 = 20.2;
oled_screw_y2 = 1.7;
oled_ribbon_x = 19.0;
oled_ribbon_x_top = 11.5;
oled_ribbon_y_offset = 3.5;
oled_ribbon_w = 4.2;
oled_ribbon_d = 13;
oled_ribbon_z = 3;
oled_ribbon_z_offset = -3;
oled_header_x = 32.2;
oled_header_y_offset = 3.5;
oled_header_z_offset = -0.4;

// Navigation switch
nav_switch_side = 5;
nav_switch_cutout_depth = 4.5;
nav_switch_height = 9;
nav_switch_cutout_size = 3.2;
nav_switch_r = 3.2;
nav_switch_r_outer = nav_switch_r + switch_clearance;
nav_switch_r_opening = 4.4;
nav_switch_cross_w = 6.4;
nav_switch_cross_d = 16;
nav_switch_cross_h = 3;
nav_switch_cutout_r = nav_switch_r + switch_clearance_small;
nav_switch_cutout_h = 1.3;
nav_switch_cutout_z = -2.3;
nav_switch_cutout_square = 1.75;
nav_switch_cutout_square_z = -4;
nav_switch_pos_x_offset = 3;
nav_switch_pos_y_offset = 2.4;
nav_switch_pos_y_offset_top = 5.4;
nav_switch_pos_y_base = 20;
nav_switch_pos_z = 2;
nav_switch_pos_h = 4;
nav_switch_cap_base_z = -1;
nav_switch_cap_base_h = 1.3 + 2.6;
nav_switch_cap_cross_z = 0.6;
nav_switch_import_z = -0.6;

// UI screw position
ui_screw_x_offset = 4.730;

// Battery holder dimensions
battery_holder_x1 = 9.6;
battery_holder_x2 = 17.4;
battery_holder_y2 = 20;
battery_holder_pos_x = 38;
battery_holder_extrude_w = 80;
battery_socket_w = 65;
battery_socket_d = 18;
battery_socket_clearance = 6;
battery_socket_wall = 2;
battery_socket_h = 20;
battery_terminal_w = 3;
battery_terminal_d = 24;
battery_terminal_neg_x = 32.8;
battery_terminal_neg_y_offset = 12;
battery_terminal_pos_x = -39.5;
battery_terminal_pos_w = 1.5;
battery_terminal_opening_x = -38;
battery_terminal_opening_z = 11;
battery_terminal_opening_h = 72;
battery_terminal_opening_r = 6;
battery_wire_conduit_x = -44;
battery_wire_conduit_y = 18;
battery_wire_conduit_z = 16;
battery_wire_conduit_h = 84;
battery_wire_conduit_r = 3;
battery_symbol_pos_x_plus = -28;
battery_symbol_pos_x_minus = 20;
battery_symbol_y = 9.4;
battery_symbol_z = 10;
battery_symbol_w = 3.2;
battery_symbol_h = 1.2;
battery_symbol_cross_offset = 1;
battery_symbol_cross_z_offset = -1.0;
battery_symbol_cross_h = 3.2;
battery_socket_d_offset = 1; // Additional offset for battery socket depth

// Speaker holder dimensions
speaker_spacing = 37;
speaker_h = 13;
speaker_pos_x1 = -11;
speaker_pos_w1 = 14;
speaker_pos_w2 = 5;
speaker_pos_w2_center = 1.5;
speaker_screw_x1 = 1;
speaker_screw_x2_offset = 1;
speaker_screw_z_offset = 6.2;
speaker_screw_h = 6;

// Chassis dimensions
chassis_x = -56;
chassis_w = 57.8;
chassis_d1 = 20;
chassis_d2 = 14;
chassis_y1 = -10;
chassis_y2 = -7;
chassis_z1 = 0;
chassis_z2 = -6;
chassis_h1 = 2;
chassis_h2 = 0.1;
chassis_cut1_x = -50;
chassis_cut1_y = -11;
chassis_cut1_w1 = 4;
chassis_cut1_w2 = 6;
chassis_cut1_d = 22;
chassis_cut1_z1 = -0.6;
chassis_cut1_z2 = 1.9;
chassis_cut1_x_offset = 2;
chassis_cut2_x = -42;
chassis_cut2_w1 = 16;
chassis_cut2_w2 = 18;
chassis_cut2_x2 = -44;
chassis_cut3_x = -56;
chassis_cut3_y = -3.5;
chassis_cut3_w = 6;
chassis_cut3_d = 7;
chassis_cut3_z = -3;
chassis_cut3_h = 5;
tp4056_pcb_x = -28.2;
tp4056_pcb_y = -9;
tp4056_pcb_w = 28.2;
tp4056_pcb_d = 18;
tp4056_pcb_z = 0.4;
tp4056_pcb_h = 2.4;
tp4056_solder_x = -28.2;
tp4056_solder_y = -8;
tp4056_solder_w = 6;
tp4056_solder_d = 16;
tp4056_solder_z = -0.6;
pico_screw_x = -48.0;
pico_screw_y = 5.68;
pico_screw_h = 6;
amp_screw_x = -54.0;
amp_screw_y = 6.2;
amp_screw_z = -3;
amp_screw_h = 6;
tp4056_stop_x = -25.8;
tp4056_stop_z = -5;
tp4056_stop_h = 5;

// Eyelet dimensions
eyelet_r = 3;
eyelet_square_offset = 3;
eyelet_square_w = 3;
eyelet_square_d = 6;
eyelet_countersunk_z = -1.2;

// Tray dimensions
tray_inner_w = 8;
tray_chassis_offset_x = 2;
tray_chassis_offset_z = -11.4;
tray_eyelet_y_offset = 3;
tray_eyelet_z = -11;
pico_usb_x_offset = 2.5;
pico_usb_z = -19.6;
pico_usb_w = 5;
pico_usb_d = 8.5;
pico_usb_h = 3.4;
pico_pcb_z = -18.2;
pico_pcb_w_clearance = 0.6;
pico_pcb_d = 22;
pico_pcb_h = 1.6;
tp4056_pcb_tray_z = -10.2;
tp4056_socket_z = -8.4;
tp4056_socket_extrude_h = 5;
tp4056_socket_hole_x = 3.6;
tp4056_socket_hole_r = 1.8;
tp4056_led_x = 7.6;
tp4056_led_r = 1.4;
bootsel_x = 13.0;
bootsel_y = 3.5;
bootsel_z = -30;
bootsel_h = 10;
bootsel_r = 1.66;
tray_bottom_screw_x = -5;
tray_bottom_screw_z_offset = 2.0;

// Shell inner supports
shell_battery_offset_x = 212.6;
shell_battery_holder_x = 112;
shell_speaker_offset_x = 13;

function center(x,y,z) = [x/-2,y/-2,z/-2];

module screw_neg(){
    ctrsnk = countersunk_height;
    
    rotate([0,90,0])
    cylinder(r = screw_r, h = neck_w,$fn=64);
    
    rotate([0,90,0])
    cylinder(ctrsnk, screw_r*2, screw_r);
    
    translate([neck_w,0,0])
    rotate([0,-90,0])
    cylinder(ctrsnk, screw_r*2, screw_r);
}

module screw_negs(){
    translate([0,neck_d*0.25,-neck_top_h-tongue_h/2])
    screw_neg();
    
    translate([0,neck_d*0.75,-neck_top_h-tongue_h/2])
    screw_neg();
}
    
module standoff(height){
    difference() {
         cylinder(r = standoff_r, h = height,$fn=64);
         cylinder(r = screw_r, h = height,$fn=64);
    };
}

module u_standoff(height, negative=false){
    if(negative){
        hull(){
         cylinder(r = standoff_r+so_clr, h = height,$fn=64);
            
         translate([0,standoff_r/2,0])
         cylinder(r = standoff_r+so_clr, h = height,$fn=64);
        }
    } else {
    difference() {
        hull(){
         cylinder(r = standoff_r, h = height,$fn=64);
            
         translate([0,standoff_r/2,0])
         cylinder(r = standoff_r, h = height,$fn=64);
        }
        
        cylinder(r = screw_r, h = height,$fn=64);
        }
    }
}

module u_standoffs(height, negative=false) {
    y1 = -ridge_d/2-screw_r;
    y2 = (pod_d*n_strings+ridge_d*n_strings)-ridge_d-standoff_r*2-screw_r;
    if(negative){
    // Lower
    translate([standoff_r,y1,0]) u_standoff(height,true);
    translate([(pod_w*n_pods_per_string)-standoff_r,y1,0]) u_standoff(height,true);
    translate([(pod_w*n_pods_per_string)/2,y1,0]) u_standoff(height,true);
    // Upper
    translate([standoff_r,y2,0]) rotate([0,0,180])u_standoff(height,true);
    translate([(pod_w*n_pods_per_string)-standoff_r,y2,0]) rotate([0,0,180]) u_standoff(height,true);
    translate([(pod_w*n_pods_per_string)/2,y2,0]) rotate([0,0,180]) u_standoff(height,true);
        
    } else {
    // Lower
    translate([standoff_r,y1,0]) u_standoff(height);
    translate([(pod_w*n_pods_per_string)-standoff_r,y1,0]) u_standoff(height);
    translate([(pod_w*n_pods_per_string)/2,y1,0]) u_standoff(height);
    // Upper
    translate([standoff_r,y2,0]) rotate([0,0,180])u_standoff(height);
    translate([(pod_w*n_pods_per_string)-standoff_r,y2,0]) rotate([0,0,180]) u_standoff(height);
    translate([(pod_w*n_pods_per_string)/2,y2,0]) rotate([0,0,180]) u_standoff(height);
    }
}

module standoffs(height, negative=false) {
    y1 = -ridge_d/2-screw_r;
    y2 = (pod_d*n_strings+ridge_d*n_strings)-ridge_d-standoff_r*2-screw_r;
    if(negative){
        // Lower
    translate([standoff_r,y1,0]) cylinder(r = standoff_r+so_clr, h = height,$fn=64);
    translate([(pod_w*n_pods_per_string)-standoff_r,y1,0]) cylinder(r = standoff_r+so_clr, h = height,$fn=64);
    translate([(pod_w*n_pods_per_string)/2,y1,0]) cylinder(r = standoff_r+so_clr, h = height,$fn=64);
    // Upper
    translate([standoff_r,y2,0]) rotate([0,0,180])cylinder(r = standoff_r+so_clr, h = height,$fn=64);
    translate([(pod_w*n_pods_per_string)-standoff_r,y2,0]) rotate([0,0,180]) cylinder(r = standoff_r+so_clr, h = height,$fn=64);
    translate([(pod_w*n_pods_per_string)/2,y2,0]) rotate([0,0,180]) cylinder(r = standoff_r+so_clr, h = height,$fn=64);
        }
       else {
    // Lower
    translate([standoff_r,y1,0]) standoff(height);
    translate([(pod_w*n_pods_per_string)-standoff_r,y1,0]) standoff(height);
    translate([(pod_w*n_pods_per_string)/2,y1,0]) standoff(height);
    // Upper
    translate([standoff_r,y2,0]) rotate([0,0,180])standoff(height);
    translate([(pod_w*n_pods_per_string)-standoff_r,y2,0]) rotate([0,0,180]) standoff(height);
    translate([(pod_w*n_pods_per_string)/2,y2,0]) rotate([0,0,180]) standoff(height);
       }
}

module countersunk(){
    cylinder(r = screw_r, h = screw_hole_depth,$fn=64);
    translate([0,0,countersunk_height])
    rotate([0,180,0])
    cylinder(h=wall_d, r1=screw_r*2, r2=screw_r);
}
