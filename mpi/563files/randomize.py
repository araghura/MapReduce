# -*- coding: utf-8 -*-
"""
Created on Thu Apr 30 02:31:03 2015

@author: Jay Ravi
"""

import random

ifile = open("list.txt")

lines = list(ifile)

ifile.close()
outlines = []

m = len(lines)
n = m
x = set()
while len(x) != m:
    num = random.randrange(0,n)
    if(num not in x):
        x.add(num)
        outlines.append(lines[num])
        
outfile = open("random.txt", "w");

outfile.writelines(outlines)

outfile.close()