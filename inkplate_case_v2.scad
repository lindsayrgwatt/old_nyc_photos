/*
Copyright Lindsay Watt 2022. Feel free to reuse for non-commercial purposes.
*/

$fa = 1;
$fs = 1;

spacer = 1;

// Global dimensions
radius = 30; /* 6mm thick exterior curve. Assume 1 unit = 0.1 mm */
height = 2370; /* 23.1 cm tall plus add 3mm to top and bottom for SD card */
width = 1800;

/*
Total height = height + 2 * radius = 2370 + 60 = 2430
Total width = width + 2 * radius = 1800 + 60 = 1860
*/

mounting_pin_radius = 15;

// Screen dimensions
screen_width = 1561; // Can make empty well wider than this. Just put 8.3mm gutter on either side
screen_height = 2174;
screen_thickness = 15;

// Active screen dimensions
active_screen_width = 1389;
active_screen_height = 2014;
active_screen_thickness = 5; // Fake variable; has no real thickness

// Circuit board dimensions
circuit_board_width = 1810; // 1 mm off in test print
circuit_board_height = 2310;
circuit_board_thickness = 10;
cavity_depth = 50;
top_and_bottom_spacers = 60; // For SD cards and button

module halfFrame() {

    cylinder(2*radius, r=radius);
    translate([0,-radius,0])
        cube([width, 2*radius, 2*radius], false);
    translate([width,0,0])
        cylinder(2*radius, r=radius);
    translate([-radius,0,0])
        cube([2*radius, height, 2*radius], false);
    translate([0,height,0])
        cylinder(2*radius, r=radius);
}

module base() {
    halfFrame();
    rotate([0,0,180])
        translate([-width,-height,0])
            halfFrame();
    cube([width, height, 2*radius]);
}


module inkplateCavity() {
    // Active screen slot
    // Use -1 to make sure actually gets cut out
    translate([206,170,-1]) // Added 0.5mm to x dimension after test print
        // Use the 2 * spacer to make sure that depth is thick enought to collide with level above
        // Otherwise have issues with adjacent but non-connected surfaces
        cube([active_screen_width, active_screen_height, active_screen_thickness + 2 * spacer]);    
    // Hidden border screen
    translate([83, 130, active_screen_thickness])  // Added 0.5mm to x dimension after test print
        cube([width - 2 * 83, screen_height, screen_thickness + 2 * spacer + 100]);
    // Circuitboard
    translate([0, 0, active_screen_thickness+screen_thickness])
        cube([circuit_board_width, circuit_board_height + top_and_bottom_spacers, circuit_board_thickness + cavity_depth]);
    // Gap for access to controls
    translate([1000,-40,20])
        cube([620, 50,41]);
}

module mountingPin() {
   cylinder(2 * radius, r=mounting_pin_radius, true);
}

difference() {
    base();
    inkplateCavity();
}

translate([32,86,0])
    mountingPin();
translate([1754,86,0 ])
    mountingPin();
translate([32, 2292,0])
    mountingPin();
translate([1754,2292,0])
    mountingPin();

