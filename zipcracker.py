import zipfile
import zlib
import argparse
import os
import time
import errno

from parseZip import getCdhEntry

def crc_check(zdata, dest_dir, file_to_check):
    prev = 0
    for line in open(exdir + "/" + file_to_check, "rb"):
        prev = zlib.crc32(line, prev)
    crc32 = struct.pack("<I",(prev & 0xffffffff))
    return True if crc32 == getCdhEntry(zdata, file_to_check)['crc32'] else False

def extract_file(zfile, zdata, pwd):
    dest_dir = "./" + os.path.splitext(zfile.filename)[0]
    try:
        f.extractall(path = dest_dir, pwd = pwd)
        for x in os.listdir(dest_dir):
            if not crc_check(zdata, dest_dir, x): raise Exception("Bad CRC!!")
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
            password = line.strip()
            if extract_file(zfile, zdata, password):
                print("[+] PASSWORD = " + str(password)[2:-1] +\
                    "\t(cracked in %.5s sec)" % str(time.time()-start))
                exit(0)
        print("[-] Password not found")
    elif mode == "brute":
        print("Not implemented yet")

if __name__ == "__main__":
    main()
