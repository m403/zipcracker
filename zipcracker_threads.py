import zipfile
import threading
import zlib
import argparse
import os
import errno
import struct
from time import time

DEBUG = False
LOCK = threading.Lock()
START_TIME = time()

class ThreadPwd(threading.Thread):
    def __init__(self, passwords, zfile):
        threading.Thread.__init__(self)
        self.passwords = passwords
        self.zfile = zfile
    def run(self):
        for line in self.passwords:
            if verify_pwd(self.zfile, line.strip()):
                success(line.strip().decode("ascii"))

def success(pwd):
    total_time = time() - START_TIME
    print("[+] PASSWORD = " + pwd + "\t(cracked in %.5s sec)" % str(total_time))

def verify_pwd(zfile, password):
    global LOCK
    LOCK.acquire()
    zfile.setpassword(password)
    try:
        result = zfile.testzip()
        LOCK.release()
        if result is None:
            return True
    except Exception as e:
        if DEBUG is True:
            print("[-] ERROR = " + str(e))
        LOCK.release()
        return False

def dict_mode(zfile, dictionary, n_threads):
    passwords = open(dictionary, 'rb').readlines()
    l = len(passwords)
    step = int(l/n_threads)
    for i in range(0, l, step):
        # last chunk
        if l < (i+step):
            bound = l
        else:
            bound = i+step
        # spawn new thread with respective chunk
        t = ThreadPwd(passwords[i:bound], zfile)
        t.start()
        print("(%s) - chunk [%d-%d]" % (t.getName(), i, bound-1))

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
    parser.add_argument("-t", dest = "n_threads", type = int, default=1,\
                        help = "specify the number of threads")
    parser.add_argument("--debug", action="store_true", default=False, help = "enable debug mode")
    args = parser.parse_args()
    DEBUG = args.debug
    return args.zname, args.dname, args.mode,args.charset,args.n_threads

def main():
    zname, dname, mode, charset, n_threads = parser()
    try:
        zfile = zipfile.ZipFile(zname)
    except Exception as e:
        print("[-] ERROR = " + str(e))
        exit(errno.ENOENT)

    if mode == "dict":
        dict_mode(zfile, dname, n_threads)
    elif mode == "brute":
        print("Not implemented yet")

if __name__ == "__main__":
    main()
