#!/usr/bin/python3

import subprocess



def run(path):
    try:
        with open(path, 'r') as file:
            source_code = file.read()
            exec(source_code, globals(), locals())
            print("Stub launched now_pendant")
    except Exception as e:
        print(f"NOW_PENDANT error\n\t{e}")


if __name__ == "__main__":
    path = "/home/schoch/dev/now_pendant/linuxcnc_comp/now_pendant.py"
    run(path)
