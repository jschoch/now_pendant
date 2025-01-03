#!/usr/bin/env python3


import gramme
import os
#import hal
import sys
import time

#print("sup")
print("Loading now pendant")
print(f"Python version: {sys.version}")

def parse_id(data):
    id = data[2]
    print(f"device id {id}")

def parse_data(data):
    msgtype = data[0]
    match msgtype:
        case True:
            c['thetest-1'] =  not c['thetest-1']
            print( "ping type was true")
        
        # data updates should match this
        case False:
            print (f"update: type was {data[1]}")
            # encoder data should be like [False, b'e', 1, -8, -9]
            updatetype = data[1]
            if updatetype == b'e':
                #encoder_list = data[b'e']
                #update_encoders( data[b'e'])
                parse_id(data)
                print("encoder data")
            elif updatetype == b'b':
                print("btn data")
            elif updatetype == b'p':
                print("pot data")
            else:
                print( ' unknown data')
        case _:
            print( "doh")

@gramme.server(8080,"192.168.11.3")
#@gramme.server(8080)
def my_awsome_data_handler(data):
    print(f"got some data\n\t{data}")
    parse_data(data)    