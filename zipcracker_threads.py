import zipfile
import threading
import zlib
import argparse
import os
import errno
import struct
from time import time

DEBUG = False
ZIP_LOCK = threading.Lock()
EXIT_LOCK = threading.Condition()
START_TIME = -1
EXIT = False
npasswd = 0

def setExit():
    global EXIT
    EXIT = True

def getExit():
    return EXIT

class ThreadPwd(threading.Thread):
    def __init__(self, passwords, zfile):
        threading.Thread.__init__(self)
        self.passwords = passwords
        self.zfile = zfile
    def run(self):
        global npasswd;
        for line in self.passwords:
            ZIP_LOCK.acquire()
            npasswd += 1
            ZIP_LOCK.release()
            if verify_pwd(self.zfile, line.strip()):
                success(line.strip().decode("ascii"))
                # acquire lock and weak up main thread
                EXIT_LOCK.acquire()
                setExit()
                EXIT_LOCK.notify()
                EXIT_LOCK.release()

def success(pwd):
    global npasswd
    total_time = time() - START_TIME
    print("[+] PASSWORD = " + pwd + "\t(cracked in %.5f sec, password per second:%.2f)" % (total_time, npasswd/total_time))

def verify_pwd(zfile, password):
    global ZIP_LOCK
    # acquire lock to set pwd and test atomically
    ZIP_LOCK.acquire()
    zfile.setpassword(password)
    try:
        result = zfile.testzip()
        ZIP_LOCK.release()
        if result is None:
            return True
    except Exception as e:
        if DEBUG is True:
            print("[-] ERROR = " + str(e))
        ZIP_LOCK.release()
        return False

def dict_mode(zfile, dictionary, n_threads):
    global START_TIME
    f = open(dictionary, 'rb')
    passwords = f.readlines()
    START_TIME = time()
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
       #print("(%s) - chunk [%d-%d]" % (t.getName(), i, bound-1))
    # wait until password is found
    EXIT_LOCK.acquire()
    while getExit() is False:
        EXIT_LOCK.wait(10)
        if threading.activeCount() == 1:
            break
    EXIT_LOCK.release()
    f.close()
    # exit not gracefully
    os._exit(os.EX_OK)

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
    zfile.close()

if __name__ == "__main__":
    main()
