#!/usr/bin/python
#
# (auto)Magically Generate Makefiles
#
__author__ = 'Jan wilmans <jw@dds.nl>'
__copyright__ = '(C) 2005 Jan Wilmans. GNU GPL 2.'
__version__ = '2.0'

import sys

#if module "makelib" cannot be found, add there path here 
sys.path.append("/home/jan/src/genmake2/")

import makelib
from makelib import *	

if __name__ == '__main__':
    print "GenMake v2.0 by Jan Wilmans"

    # start of project configuration
    makelib.Module = makelib.ChessModule

    my_project = Project("nowind msx emulator")

    main = makelib.JawModule("main")
    main.recursionEnabled = True
    my_project.addModule(main)
    my_project.generate()
    #todo: implement the default-build setting
    # the choice for debug/release build should be configured in the toplevel makefile
    # how this is passed down is unknown yet.. maybe an environment variable should specify this.


