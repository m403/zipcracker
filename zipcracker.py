import zipfile, argparse
import os, zlib
import time
import errno
from parseZip import *

def crcCheck(d, f):
    prev = 0
    for line in open("./extracted/"+f, "rb"):
        prev = zlib.crc32(line, prev)
    crc32 = struct.pack("<I",(prev & 0xffffffff))
    return True if crc32 == getCdhEntry(d, f)['crc32'] else False

def extractfile(f, d, p):
    try:
        f.extractall(path = "./extracted",pwd = p)
        for x in os.listdir("./extracted"):
            if not crcCheck(d, x): raise Exception("Bad CRC!!")
        return True
    except: return False

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

    # BEAUTIFY ME!!!!
    if ((args.zname == None) or (args.dname == None and args.mode == "dict")) or \
       (args.mode != "dict" and args.mode != "brute") or \
       (args.mode == "brute" and args.charset == None):
        parser.print_usage()
        exit(errno.EINVAL)

    zname = args.zname
    dname = args.dname
    mode = args.mode
    charset = args.charset

    try:
        zfile = zipfile.ZipFile(zname)
        zdata = open(zname,"rb").read()
    except Exception as e:
        print("[-] ERROR = " + str(e))
        exit(errno.ENOENT)

    if mode == "dict":
        passfile = open(dname, 'rb').readlines()
        for line in passfile:
            if line[:2] != bytes("#!","ascii"):
                password = line.strip()
                if extractfile(zfile, zdata, password):
                    print("[+] PASSWORD = " + str(password)[2:-1] +\
                        "\t(cracked in %.5s sec)" % str(time.time()-start))
                    exit(0)
        print("[-] Password not found")
    elif mode == "brute":
        print("Not implemented yet")

if __name__ == "__main__":
    main()
