#!/usr/bin/env python3


import gramme
import os
import hal
import sys
import time
import linuxcnc
from dataclasses import dataclass, asdict


noupdates = 1

sys.path.append("/home/schoch/dev/now_pendant/linuxcnc_comp")
from Npd import Npd

#print("sup")
print("Loading now pendant")
print(f"Python version: {sys.version}")


pinmap = [4,5,6,7,8,9] # 
btnpinmap = [0,1]
dacpinmap = [3]  # DAC fixed
nout = 5
num_btns = 2

encoder_map = {
    0: 0.0,
    1: 1.0, 
    2: 0.5, 
    3: 0.25, 
    4: 0.01, 
    5: 0.0025}

pot_map = {
    0: 0.0,
    1: 0.005,
    2: 0.01,
    3: 0.05,
    4: 0.1,
    5: 0.5,
    6: 0.75,
    7: 1,
    8: 1.25,
    9: 1.4
}

c = hal.component("now_pendant")
c.newpin("analog-out-%02d" % dacpinmap[0], hal.HAL_FLOAT, hal.HAL_IN)
c.newparam("analog-out-%02d-offset" % dacpinmap[0], hal.HAL_FLOAT, hal.HAL_RW)
c.newparam("analog-out-%02d-scale" % dacpinmap[0], hal.HAL_FLOAT, hal.HAL_RW)
c['analog-out-%02d-scale' % dacpinmap[0]] = 1.0

for port in range(6):
    c.newpin("analog-in-%02d" % port, hal.HAL_FLOAT, hal.HAL_OUT)
    c.newparam("analog-in-%02d-offset" % port, hal.HAL_FLOAT, hal.HAL_RW)
    c.newparam("analog-in-%02d-gain" % port, hal.HAL_FLOAT, hal.HAL_RW)
    
    c['analog-in-%02d-gain' % port] = 1.0


# Mapped pot, range 1..10, but could be 1..(size of S32)

for port in range(1):
    c.newpin("mpot-in-%02d" % port, hal.HAL_S32, hal.HAL_OUT) 
    #c.newparam("mpot-in-%02d" % port, hal.HAL_S32, hal.HAL_OUT)

    
for port in range(nout):
    c.newpin("digital-out-%02d" % port, hal.HAL_BIT, hal.HAL_IN)
    c.newparam("digital-out-%02d-invert" % port, hal.HAL_BIT, hal.HAL_RW)

for port in range(num_btns):
    c.newpin("digital-in-%02d" % port, hal.HAL_BIT, hal.HAL_OUT)
    c.newpin("digital-in-%02d-not" % port, hal.HAL_BIT, hal.HAL_OUT)
c.newpin("network-error", hal.HAL_BIT, hal.HAL_IN)

for i in range(5):
    c.newpin("mpg-count-%02d" % i, hal.HAL_S32, hal.HAL_IN)

c.newpin("jog-scale",hal.HAL_FLOAT, hal.HAL_IN)
c.newpin("thetest-1",hal.HAL_BIT, hal.HAL_IN)

c['jog-scale'] = encoder_map[5]

c.ready()
print("now_pendant pin setup done")
noupdates = 0


#############################################################################################################################################################
#
#
#  "server" logic
#
######################################################################################################################################################

class MsgType:
    PING = 7
    STATE = 5
    ERRORS = 2
    POS = 3

#class State:
#    def __init__(self, system, machine, motion, homed):
#        self.system = system
#        self.machine = machine
#        self.motion = motion
#        self.homed = homed 

@dataclass
class Ping:
    sequence_number: int

@dataclass
class State:
    system: int
    machine: int
    motion: int
    homed: int

@dataclass
class Amsg:
    msg_type: MsgType
    state: State

@dataclass
class Errors:
    errornumber: int
    msg: str  # Python's str is equivalent to C++'s String

@dataclass
class Pos:
    x: float = 0.0
    z: float = 0.0 
    x_dtg: float = 0.0
    z_dtg: float = 0.0
    jog_scale: float   = 0.0# Renamed to follow Python conventions
    g5x: int = 0
    x_jog_counts: int = 0 # Renamed to follow Python conventions
    z_jog_counts: int = 0

def updatePos():
    global stat
    # poll should already have recently run, updateState i think

    # actual position should be like this
    # current trajectory position, (x y z a b c u v w) in machine units. 

    spos = stat.actual_position
    print(f"spos: {spos}")
    pos = Pos()
    pos.x = spos[0]
    pos.z = spos[2]
    pos.x_jog_counts = hal.get_value("axis.x.jog-counts")
    pos.z_jog_counts = hal.get_value("axis.z.jog-counts")
    pos.jog_scale = hal.get_value("axis.x.jog-scale")
    print(f"pos: {pos}")
    pos_dict = asdict(pos) 
    pos_array = list(pos_dict.values())
    data = [MsgType.POS,pos_array]
    return data

def test_changed(obj,data):
    print(" got a flip flop")
    print(data)

ticker = 0

def scale_ticker(ticker,dir):
    if(dir):
        if(ticker < 5):
            ticker = ticker + 1
        else:   
            ticker = 5
    else:
        if(ticker > 0):
            ticker = ticker - 1
        else:   
            ticker = 0
    print(f' ticker: {ticker}')
    return ticker



sel_prev = 0
stat = linuxcnc.stat()


updating = 0
#def update_encoders(encoder_list):
def update_encoders(data_dict):

    # TODO: if the pendant resets the counts will be different 
    #        if the hal component resets the counts will be different.  
    #       this could lead to violent jogging, need to fix
    global updating 
    global noupdates
    if(updating or noupdates):
        print(f"skip {updating} {noupdates}")
        return;
    updating = 1
    global ticker
    global sel_prev
    #print(encoder_list)
    #c['mpg-count-%02d'% encoder_list[0]] = encoder_list[2]
    enc_id = data_dict['device_id']
    enc_count = data_dict['count']
    print (f"updating encoder {enc_id} t: {type(enc_id)} count: {enc_count} t{type(enc_count)}")
    c['mpg-count-%02d'% enc_id]   = data_dict['count']
    if(enc_id == 4):
        
        prev_count = data_dict['prev_count']
        count = data_dict['count']
        diff = sel_prev - count

        if(diff > 8 and diff > 0):
            ticker = scale_ticker(ticker,0)
            c['jog-scale'] = encoder_map[ticker]
            sel_prev = count
        elif(diff < -8 and diff < 0):
            ticker = scale_ticker(ticker,1)
            c['jog-scale'] = encoder_map[ticker]
            sel_prev = count 
    updating = 0

def updateButtons(data_dict):
    global updating 
    global noupdates
    btn_id = data_dict['device_id']
    print(f"btn pressed {btn_id}")
    c["digital-in-%02d" % btn_id] = data_dict['state']

def updatePots(data_dict):
    pot_id = data_dict['device_id']
    v = data_dict['map_value']
    print(f"maybe pot {pot_id} changed: {v}")
    c["mpot-in-%02d" %pot_id] = v
    if(pot_id == 0):
        c['jog-scale'] = pot_map[v]
        None

def updateState():
    global stat
    stat.poll()

    print(f"lcnc estop\n\t{stat.estop}")
    print(f"lcnc homed\n\t{stat.homed}")
    state_msg = State(0,stat.estop,0,0)
    
    #state_msg = State(
    #    system=c["rio.sys-status"], 
    #    machine=c["iocontrol.0.user-enable-out"],
    #    #motion=c[""], 
    #    motion=0, #  not sure what to put here
    #    home=c["motion.is-all-homed"])

    print(f"Update state {state_msg}")

    data = [
        7,[
            state_msg.system,
            state_msg.machine,
            state_msg.motion,
            state_msg.homed
            ]
    ]
    return data,state_msg

client = gramme.client(host="192.168.11.4", port=8080)
@gramme.server(8080)
def my_awsome_data_handler(data):
    try:
        print( data)
        if(data[0] == True):
            print("got ping msg")
            data,s = updateState()
            client.send( data )
            if not s.machine:
                pos = updatePos()
                client.send(pos)

        else:
            npd = Npd(data)
            data_dict = npd.parse_data()
            print(f"parsed data was\n\t{data_dict}")
            if data_dict['device_type'] == "enc":
                print("got to update encoders")
                update_encoders(data_dict)
                data = updatePos()
                client.send(data)
            if data_dict['device_type'] == 'btn':
                updateButtons(data_dict)
            if data_dict['device_type'] == 'pot':
                updatePots(data_dict)
            else:
                print("Unknown data type")
            
            #data = {'cmd': 1}
            #data = ['12',22]
            #data = {1,'some crap who knows'}
            #data = [5,"this is a load of crap"]
            #data = [5,[1,2,3,16]]
            state_msg = State(system=1, machine=2, motion=3, homed=4)
            client.send(data)
        return 0
    except KeyboardInterrupt:
        raise SystemExit
    except Exception as e:
        print(f"exception: {e}")
        #raise SystemExit


def nothing():
    return
################################################################
#                    MAIN LOOP                                 #
################################################################    
if __name__ == "__run__":
    try:
        print("Starting now_pendant loop")
        while 1:
            time.sleep(1)
    #except (KeyboardInterrupt,):
    except KeyboardInterrupt:
        print("keyboard interrupt, exiting")
        raise SystemExit
    except Exception as e:
        print(f"exception {e}")
        raise SystemExit
    except:
        print("2nd except, but what does this actually do?")
        pass
    finally:
        print("final final, exiting via c.exit")
        c.exit()
