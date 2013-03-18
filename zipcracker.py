import zipfile
import zlib
import argparse
import os
import errno
import struct
from time import time

from parseZip import getCdhEntry

DEBUG = False

def crc_check(zdata, dest_dir, file_to_check):
    prev = 0
    for line in open(dest_dir + "/" + file_to_check, "rb"):
        prev = zlib.crc32(line, prev)
    crc32 = struct.pack("<I",(prev & 0xffffffff))
    return True if crc32 == getCdhEntry(zdata, file_to_check)['crc32'] else False

def extract_file(zfile, zdata, password):
    dest_dir = "./" + os.path.splitext(zfile.filename)[0]
    try:
        zfile.extractall(path = dest_dir, pwd = password)
        for x in os.listdir(dest_dir):
            if not crc_check(zdata, dest_dir, x): raise Exception("Bad CRC!!")
        return True
    except Exception as e:
        if DEBUG is True:
            print("[-] ERROR = " + str(e))
        return False

def success(start_time, pwd):
    total_time = time() - start_time
    print("[+] PASSWORD = " + pwd + "\t(cracked in %.5s sec)" % str(total_time))

def dict_mode(zfile, zdata, dictionary):
    passwords = open(dictionary, 'rb').readlines()
    for pwd in passwords:
        if extract_file(zfile, zdata, pwd.strip()):
            return pwd.strip().decode("ascii")
    #return(''.join([pwd.strip().decode("ascii")
                    #for pwd in passwords if extract_file(zfile, zdata, pwd.strip())]))

def parser():
    parser = argparse.ArgumentParser(description = "Zip file cracker",version="1.0")
    parser.add_argument("-f", dest = 'zname',type = str,\
                      help = "specify zip file")
    parser.add_argument("-d", dest = 'dname',type = str,\
                      help = "specify dictionary file")
    parser.add_argument("-m", dest = "mode", type = str,\
                      help = "specify the operation mode dict|brute")
    parser.add_argument("-c", dest = "charset", type = str,\
                      help = "specify the charset used in the bruteforce mode")
    args = parser.parse_args()

    # BEAUTIFY ME!!!!
    if ((args.zname == None) or (args.dname == None and args.mode == "dict")) or \
       (args.mode != "dict" and args.mode != "brute") or \
       (args.mode == "brute" and args.charset == None):
        parser.print_usage()
        exit(errno.EINVAL)

    return args.zname, args.dname, args.mode,args.charset



def main():
    start = time()

    zname, dname, mode, charset = parser()
    pwd = ''
    try:
        zfile = zipfile.ZipFile(zname)
        zdata = open(zname,"rb").read()
    except Exception as e:
        print("[-] ERROR = " + str(e))
        exit(errno.ENOENT)

    if mode == "dict":
        pwd = dict_mode(zfile, zdata, dname)
    elif mode == "brute":
        print("Not implemented yet")

    if pwd:
        success(start, pwd)
    else:
        print("[-] Password not found")

if __name__ == "__main__":
    main()
