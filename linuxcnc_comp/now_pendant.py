#!/usr/bin/env python3


import gramme
import os
import hal
import sys
import time


noupdates = 1

sys.path.append("/home/schoch/dev/now_pendant/linuxcnc_comp")
from Npd import Npd

#print("sup")
print("Loading now pendant")
print(f"Python version: {sys.version}")


pinmap = [4,5,6,7,8,9] # 
dacpinmap = [3]  # DAC fixed
nout = 5

encoder_map = {
    0: 0.0,
    1: 1.0, 
    2: 0.5, 
    3: 0.25, 
    4: 0.01, 
    5: 0.0025}

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
    
for port in range(nout):
    c.newpin("digital-out-%02d" % pinmap[port], hal.HAL_BIT, hal.HAL_IN)
    c.newparam("digital-out-%02d-invert" % pinmap[port], hal.HAL_BIT, hal.HAL_RW)
for port in range(nout, 6):
    c.newpin("digital-in-%02d" % pinmap[port], hal.HAL_BIT, hal.HAL_OUT)
    c.newpin("digital-in-%02d-not" % pinmap[port], hal.HAL_BIT, hal.HAL_OUT)
    c.newparam("digital-in-%02d-pullup" % pinmap[port], hal.HAL_BIT, hal.HAL_RW)
c.newpin("network-error", hal.HAL_BIT, hal.HAL_IN)
for i in range(5):
    c.newpin("mpg-count-%02d" % i, hal.HAL_S32, hal.HAL_IN)

c.newpin("jog-scale",hal.HAL_FLOAT, hal.HAL_IN)
c.newpin("thetest-1",hal.HAL_BIT, hal.HAL_IN)
c.ready()
print("now_pendant pin setup done")
noupdates = 0

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


@gramme.server(8080)
def my_awsome_data_handler(data):
    try:
        print( data)
        npd = Npd(data)
        data_dict = npd.parse_data()
        print(f"parsed data was\n\t{data_dict}")
        if data_dict['device_type'] == "enc":
            print("got to update encoders")
            update_encoders(data_dict)
        if data_dict['device_type'] == 'btn':
            updateButtons(data_dict)
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
if __name__ == "__main__":
    try:
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
