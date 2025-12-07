include <Diapasonix.scad>

module headers_cover() {
    r = cap_header_cover_r;
    dist = cap_header_cover_dist;
    gr_l = cap_header_cover_groove_l;
    
    difference(){
        translate([r, r, 0])
        hull(){
            sphere(r);
            translate([0, dist, 0])
            sphere(r);
        }
        
        // Inner groove
        translate([r - (header_w / 2), (dist - gr_l) / 2 + r, 0])
            cube([header_w, gr_l, groove_depth]);
        
        // Remove bottom
        translate([0, 0, cap_header_cover_bottom_z])
        cube([neck_w, neck_d, cap_header_cover_bottom_h]);
        
        // Remove top
        translate([0, 0, cap_header_cover_top_z])
        cube([r * 2, dist + r * 2, cap_header_cover_top_h]);
        
        // Screws
        translate([r, cap_header_cover_screw_y1, 0])
        countersunk();
        
        translate([r, dist + cap_header_cover_screw_y2_offset, 0])
        countersunk();
    }
}

if(is_undef(is_root)) {
    headers_cover();
}