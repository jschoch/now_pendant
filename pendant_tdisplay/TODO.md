1. use the custom datatypes to deal with msgs from lcnc

```


send msg type to allow for parsing of the 2nd value in the array based on msg_type

ping should be small.
can skip state changes if none.

struct CustomClass {
    int msg_type;
    Custom msg;
    MSGPACK_DEFINE(i, f, s); // -> [i, f, s]
};

enum{
    LCMD_PING,
    LCMD_IDLE,
    LCMD_MDI,
    LCMD_AUTO
} lcmd_t

enum{
    ... the lcnc machine state
}

struct LCNC_STATE {
    lcmd_t cmd,
    float position_scale, // is this needed if we are using jog-scale-enable as guard to sending enc updates?
    float feed_override,
    float rapid_override,
    float x_pos,
    float z_pos,
    float x_dtg,
    float z_dtg,
    lstate machine_state  // IDLE, RUNNING,PAUSED, ESTOP
    lsstate system_state  // powered,fault and whatnot
    mstate motion_state // absolute, relative, 
    work work_coordinates // P0..N  or G53 G54?
    modals  modal  //  maybe a list of modals that can be split like "G90,G33,G7"
}

```

2. detect operation state changes and change screen accordingly.  BG color should be strong signal to user
3. get some data from lcnc to test with.
4. write some tests


# Done:



# not doing
1. try to connect to wifi at the same time as esp_now for testing.



