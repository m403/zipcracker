import zipfile
import zlib
import argparse
import os
import errno
import struct
from time import time

DEBUG = False

def verify_pwd(zfile, password):
    zfile.setpassword(password)
    try:
        if zfile.testzip() is None:
            return True
    except Exception as e:
        if DEBUG is True:
            print("[-] ERROR = " + str(e))
        return False

def success(start_time, pwd):
    total_time = time() - start_time
    print("[+] PASSWORD = " + pwd + "\t(cracked in %.5s sec)" % str(total_time))

def dict_mode(zfile, dictionary):
    f = open(dictionary, 'rb')
    passwords = f.readlines()
    for pwd in passwords:
        if verify_pwd(zfile, pwd.strip()):
            f.close()
            return pwd.strip().decode("ascii")
    f.close()

def parser():
    global DEBUG
    parser = argparse.ArgumentParser(description = "Zip file cracker",version="1.0")
    parser.add_argument("-f", dest = 'zname',type = str, required = True,\
                        help = "specify zip file")
    parser.add_argument("-d", dest = 'dname',type = str,\
                        help = "specify dictionary file")
    parser.add_argument("-m", dest = "mode", type = str, required = True,\
                        choices = ['dict','brute'], help = "specify the operation mode: dict or brute")
    parser.add_argument("-c", dest = "charset", type = str,\
                        help = "specify the charset used in the bruteforce mode")
    parser.add_argument("--debug", action="store_true", default=False, help = "enable debug mode")
    args = parser.parse_args()
    DEBUG = args.debug

    return args.zname, args.dname, args.mode,args.charset

def main():
    start = time()

    zname, dname, mode, charset = parser()
    pwd = ''
    try:
        zfile = zipfile.ZipFile(zname)
    except Exception as e:
        print("[-] ERROR = " + str(e))
        exit(errno.ENOENT)

    if mode == "dict":
        pwd = dict_mode(zfile, dname)
    elif mode == "brute":
        print("Not implemented yet")

    if pwd:
        success(start, pwd)
    else:
        print("[-] Password not found")
    zfile.close()

if __name__ == "__main__":
    main()
