import zipfile
import time
import argparse
import sys
import os
import zlib
from parseZip import *

def crcCheck(zf,f):
    prev = 0
    for line in open(f,"rb"):
        prev = zlib.crc32(line, prev)
    crc32 = "%X"%(prev & 0xffffffff)

    try:
        fz = open("zippo.zip","rb").read()
    except:
        print("fuffa")

    print(bytes(crc32,"ascii"))
    print(getCdhEntry(fz,f)['crc32'])

    if bytes(crc32,"ascii") == getCdhEntry(fz,f)['crc32']:
        print("FOUND!!!!!")
        return True
    return False

def extractfile(f,p):
    try:
        f.extractall(path = "./extracted",pwd = bytes(p,'ascii'))
        for x in os.listdir("./extracted"):
            if crcCheck(f.filename,x) == False:
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
    except Exception as e:
        print("[-] ERROR = " + str(e))
        exit(0)

    if mode == "dict":
        passfile = open(dname, 'r')
        for line in passfile.readlines():
            password = line.strip("\r\n")
            guess = extractfile(zfile,password)
            if guess:
                print("[+] PASSWORD = " + password +\
                    "\t(cracked in %.5s sec)" % str(time.time()-start))
                exit(0)
        print("[-] Password not found")
    elif mode == "brute":
        print("Not implemented yet")

if __name__ == "__main__":
    main()
