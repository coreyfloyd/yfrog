#!/usr/bin/env python

import imageshack
import sys

if __name__ == "__main__":

    if len(sys.argv)!=3:
        print "Usage upload.py <devkey> <filename/url>"
        sys.exit(1)
    u = imageshack.Uploader(sys.argv[1])

    try:
        if sys.argv[2].startswith("http://"):
            print u.uploadURL(sys.argv[2])
        else:
            print u.uploadFile(sys.argv[2])
    except imageshack.ServerException, e:
        print str(e)
