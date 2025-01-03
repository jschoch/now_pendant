#!/usr/bin/env python3


import gramme
import os
#import hal
import sys
import time
sys.path.append("../linuxcnc_comp")
from Npd import Npd

#print("sup")
print("Loading now pendant")
print(f"Python version: {sys.version}")



@gramme.server(8080,"192.168.11.3")
#@gramme.server(8080)
def my_awsome_data_handler(data):
    npd = Npd(data,True)
    print(f"got some data\n\t{data}")
    data_dict = npd.parse_data()    
    print(f"parsed data was\n\t{data_dict}")