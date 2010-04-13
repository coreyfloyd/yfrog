#!/usr/bin/env python

import yfrog
import sys

if __name__ == "__main__":

    if len(sys.argv)<4:
        print "Usage upload.py <username> <password> <filename/url> [<text>] [<source>]"
        sys.exit(1)
    u = yfrog.Uploader()
    if len(sys.argv)>=5:
        msg = sys.argv[4]
    else:
        msg = None
    if len(sys.argv)>=6:
        src = sys.argv[5]
    else:
        src = 'yfrog'

    try:
        if sys.argv[3].startswith("http://"):
            print u.uploadURL(sys.argv[3],sys.argv[1],sys.argv[2],message=msg,source=src)
        else:
            print u.uploadFile(sys.argv[3],sys.argv[1],sys.argv[2],message=msg,source=src)
    except yfrog.ServerException, e:
        print str(e)
