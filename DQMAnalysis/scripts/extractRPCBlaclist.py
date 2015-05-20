#!/usr/bin/env python

import sys, os
from ROOT import *

minEvents = 100000
minDeadFrac = 0.8

basePath = "/store1/jhgoh/RPC/DQM/SingleMu_2012D"

for fName in os.listdir(basePath):
    if not fName.startswith("DQM"): continue
    if not fName.endswith(".root"): continue

    runNumber = int(fName.split('_')[2][-6:])
    folderName = "/DQMData/Run %d/RPC/Run summary/AllHits" % runNumber

    f = TFile(os.path.join(basePath, fName))
    if f == None:
        print "Cannot open file", fName
        continue
    d = f.Get(folderName)
    if d == None:
        print "Cannot open directory", folderName
        continue

    nRPCEvents = d.Get("RPCEvents").GetBinContent(1)
    if nRPCEvents < minEvents: continue

    print runNumber

    for wheel in range(-2, 3):
        dirWheel = "Barrel/Wheel_%d" % wheel
        for sector in range(1, 13):
            for station in range(1, 5):
                dd = d.Get("%s/sector_%d/station_%d" % (dirWheel, sector, station))
                for hName in [x.GetName() for x in dd.GetListOfKeys()]:
                    if not hName.startswith("Occupancy_"): continue
                    h = dd.Get(hName)
                    if h == None:
                    #    print "cannot find", hName
                        continue
                    if not h.IsA().InheritsFrom("TH1"): continue

                    chName = hName[10:]
                    rollSuffix = []
                    if "RB2in" in chName: suffixes = ["B", "M", "F"]# RB2in is divide by 3 rolls
                    else: suffixes = ["B", "F"]

                    nstrip = h.GetNbinsX()
                    nPerRoll = nstrip/len(suffixes)

                    for iroll, suffix in enumerate(suffixes):
                        rollName = chName+"_"+suffix

                        ndead = 0.
                        for istrip in range(nPerRoll):
                            bin = 1+istrip+nPerRoll*iroll
                            if h.GetBinContent(bin) == 0: ndead += 1
                        fdead = ndead/nPerRoll
                        if fdead >= minDeadFrac:
                            print "%s %f" % (rollName, fdead)

    for disk in range(-4, 5):
        if disk == 0: continue
        if disk > 0: dirDisk = "Endcap+/Disk_%d" % disk
        else: dirDisk = "Endcap-/Disk_%d" % disk
        for ring in range(2, 4):
            for sector in range(1, 7):
                for segment in range(6*(sector-1)+1, 6*sector+1):
                    chName = "RE%+d_R%d_CH%02d" % (disk, ring, segment)
                    hName = "%s/ring_%d/sector_%d/Occupancy_%s" % (dirDisk, ring, sector, chName)
                    h = d.Get(hName)
                    if h == None:
                    #    print "Cannot find", hName
                        continue
                    if not h.IsA().InheritsFrom("TH1"): continue

                    suffixes = ["A", "B", "C"]

                    nstrip = h.GetNbinsX()
                    nPerRoll = nstrip/3; # Endcap chambers are consist of 3 rolls

                    for iroll, suffix in enumerate(suffixes):
                        rollName = chName+"_"+suffix

                        ndead = 0.
                        for istrip in range(nPerRoll):
                            bin = 1+istrip+nPerRoll*iroll
                            if h.GetBinContent(bin) == 0: ndead += 1
                        fdead = ndead/nPerRoll
                        if fdead > minDeadFrac:
                            print "%s %f" % (rollName, fdead)

