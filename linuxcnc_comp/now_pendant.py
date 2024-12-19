#!/usr/bin/env python3


import gramme
import os
#from gi.repository import GObject
#from gi.repository import GLib
import hal
#from hal_glib import GStat
#GSTAT = GStat()
import sys
import time

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
    3: 0.1, 
    4: 0.05, 
    5: 0.01}

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

# connect a GSTAT message to a callback function
#GSTAT.connect("mode_manual",test_changed)
#GSTAT.forced_update()


sel_prev = 0

def update_encoders(encoder_list):
    global ticker
    global sel_prev
    #print(encoder_list)
    c['mpg-count-%02d'% encoder_list[0]] = encoder_list[2]
    if(encoder_list[0] == 4):
        
        prev_count = encoder_list[1]
        count = encoder_list[2]
        diff = sel_prev - count
        #print("update scale")
        #print(f'ticker {ticker} diff{diff}')
        if(diff > 8 and diff > 0):
            ticker = scale_ticker(ticker,0)
            c['jog-scale'] = encoder_map[ticker]
            sel_prev = count
        elif(diff < -8 and diff < 0):
            ticker = scale_ticker(ticker,1)
            c['jog-scale'] = encoder_map[ticker]
            sel_prev = count 
        #print("ticker %d" % ticker)
        #print(encoder_map[ticker])

@gramme.server(8080)
def my_awsome_data_handler(data):
    print( data)
    #print( bin(data))
    match data:
        case {b'h':True}:
            c['thetest-1'] =  not c['thetest-1']
            print( "true")
        case {b'h':False}:
            print ('got some data')
            if b'e' in data:
                #encoder_list = data[b'e']
                update_encoders( data[b'e'])
            else:
                print( ' didnt find encoders')
        case _:
            print( "doh")


def nothing():
    return
################################################################
#                    MAIN LOOP                                 #
################################################################    
if __name__ == "__main__":
    try:
        while 1:
            #GLib.MainLoop().run()
            time.sleep(1)
    except (KeyboardInterrupt,):
        #raise SystemExit, 0
        raise SystemExit
    finally:
        c.exit()
