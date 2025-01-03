#!/usr/bin/env python3


import gramme
import os
#import hal
import sys
import time

#print("sup")
print("Loading now pendant")
print(f"Python version: {sys.version}")

@gramme.server(8080,"192.168.11.3")
#@gramme.server(8080)
def my_awsome_data_handler(data):
    print("got some data")
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
                #update_encoders( data[b'e'])
                print("encoder data")
            else:
                print( ' didnt find encoders')
        case _:
            print( "doh")