#!/usr/bin/python3

import subprocess



def run(path):
    try:
        result = subprocess.run(["python3",path], check=True)
        print(f"Ran {path}")
    except subprocess.CalledProcessError as e:
        print(f"NOW_PENDANT error\n\t{e}")


if __name__ == "__main__":
    path = "/home/schoch/dev/now_pendant/linuxcnc_comp/now_pendant.py"
    run(path)
