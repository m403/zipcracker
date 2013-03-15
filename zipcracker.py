import zipfile
import time
import argparse
import os
import zlib
from parseZip import *

# FIXME for zip with more file than one
def crcCheck(d, f):
    prev = 0
    for line in open("./extracted/"+f, "rb"):
        prev = zlib.crc32(line, prev)
    crc32 = struct.pack("<I",(prev & 0xffffffff))

    if crc32 == getCdhEntry(d, f)['crc32']:
        return True
    else:
        return False

def extractfile(f, d, p):
    try:
        f.extractall(path = "./extracted",pwd = bytes(p,'ascii'))
        for x in os.listdir("./extracted"):
            if crcCheck(d, x) == False:
                return
        return p
    except:
        return

def main():
    start = time.time()

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

    if (args.zname == None) or (args.dname == None and args.mode == "dict"):
        parser.print_usage()
        exit(0)

    if args.mode != "dict" and args.mode != "brute":
        parser.print_usage()
        exit(0)

    if args.mode == "brute" and args.charset == None:
        print("[-] ERROR = please, specify a charset for the bruteforcer!!")
        exit(0)

    zname = args.zname
    dname = args.dname
    mode = args.mode
    charset = args.charset

    try:
        zfile = zipfile.ZipFile(zname)
        zdata = open(zname,"rb").read()
    except Exception as e:
        print("[-] ERROR = " + str(e))
        exit(0)

    if mode == "dict":
        passfile = open(dname, 'r')
        for line in passfile.readlines():
            password = line.strip("\r\n")
            guess = extractfile(zfile, zdata, password)
            if guess:
                print("[+] PASSWORD = " + password +\
                    "\t(cracked in %.5s sec)" % str(time.time()-start))
                exit(0)
        print("[-] Password not found")
    elif mode == "brute":
        print("Not implemented yet")

if __name__ == "__main__":
    main()
