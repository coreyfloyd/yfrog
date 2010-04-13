#!/usr/bin/env python

import os
import sys
import getopt

import imageshack


def usage():
    print "Usage apitest.py --file filename --key key [--id ID] [--cookie COOKIE] [--tags TAGS] [--visibility PUBLIC] [--username USERNAME] [--password PASSWORD] [--blocksize=BLOCKSIZE]  [--numblocks=NUMBLOCKS]"

def getopts(argv):
    
    data = { 'file': None,
             'key': None,
             'cookie': None,
             'id': None,
             'tags': None,
             'username': None,
             'password': None,
             'public': None }
    try:
        opts, args = getopt.getopt(sys.argv[1:], "f:k:i:t:v:u:p:b:n:", ["file=","key=","id=","cookie=","tags=","visibility=","username=","password=","blocksize=","numblocks="])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(2)
    for o, a in opts:
        if o in ("-f", "--file"):
            data['file'] = a
        if o in ("-k", "--key"):
            data['key'] = a
        elif o in ("-i", "--id"):
            data['id'] = a
        elif o in ("-c", "--cookie"):
            data['cookie'] = a
        elif o in ("-t", "--tags"):
            data['tags'] = a
        elif o in ("-v", "--visibility"):
            data['public'] = True if a.upper() == 'TRUE' else False 
        elif o in ("-u", "--username"):
            data['username'] = a
        elif o in ("-p", "--password"):
            data['password'] = a
        elif o in ("-b", "--blocksize"):
            data['blocksize'] = int(a)
        elif o in ("-n", "--numblocks"):
            data['numblocks'] = int(a)
    return data

def main():
    data = getopts(sys.argv)
    if not data['key']:
        print 'ERROR: No developer key specified'
        sys.exit(1)
    if not data['file'] or not os.path.isfile(data['file']):
        print 'ERROR: No file specified or not existing file'
        sys.exit(1)
    uploader = imageshack.ChunkedUploader(data['key'], data['cookie'],
                                          data['username'], data['password'])
    try:
        res = uploader.upload_file(data['file'], data['tags'], data['public'])
    except Exception as e:
        print 'ERROR: File could not be uploaded:'
        print e
        sys.exit(1)
    print res[2]
        
    
if __name__ == "__main__":
    main()
