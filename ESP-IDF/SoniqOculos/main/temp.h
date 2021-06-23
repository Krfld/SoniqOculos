//char *setup_msg = "mode:volume:m_devices:m_equalizer:r_sd_state:r_bc_state";

char *m = "m 0";      // Mode
char *v = "v 100";    // Volume
char *d = "d 0";      // Devices
char *eq = "e 2 2 2"; // EQ
char *sd = "s 0";     // SD
char *bc = "b 0";     // BCD

char *mode = "m [0,1]";

char *volume = "v [0..100]";

char *devices = "d [0,1,2]";

char *equalizer = "e b[1,2,3] m[1,2,3] t[1,2,3]";

char *sd_state = "sd [0,1]";

char *bc_state = "bc [0,1]";