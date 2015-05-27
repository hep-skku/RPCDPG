#!/usr/bin/env python

import sys, os
from ROOT import *

minEvents = 10000
minDeadFrac = 0.8

basePath = "/store1/jhgoh/RPC/DQM/SingleMu_2012D"
#basePath = "/store1/jhgoh/RPC/DQM/Cosmics_2015"

blacklist = {}
nRun = 0

for fName in os.listdir(basePath):
    if not fName.startswith("DQM"): continue
    if not fName.endswith(".root"): continue

    runNumber = int(fName.split('_')[2][-6:])
    folderName = "/DQMData/Run %d/RPC/Run summary/AllHits" % runNumber
    effFolderName = "/DQMData/Run %d/RPC/Run summary/RPCEfficiency" % runNumber

    f = TFile(os.path.join(basePath, fName))
    if f == None:
        print "Cannot open file", fName
        continue
    d = f.Get(folderName)
    if d == None:
        print "Cannot open directory", folderName
        continue

    nRPCEvents = d.Get("RPCEvents").GetBinContent(1)
    if nRPCEvents < minEvents:
        print "Too small statistics, ", nRPCEvents, fName
        continue
    nRun += 1

    print runNumber

    for wheel in range(-2, 3):
        dirWheel = "Barrel/Wheel_%d" % wheel
        for sector in range(1, 13):
            if wheel > 0:
                hEff = f.Get("%s/Efficiency_Roll_vs_Sector_Wheel_%+d" % (effFolderName, wheel))
                if hEff == None: hEff = f.Get("%s/Efficiency_Roll_vs_Sector_Wheel_%d" % (effFolderName, wheel))
            else: hEff = f.Get("%s/Efficiency_Roll_vs_Sector_Wheel_%d" % (effFolderName, wheel))
            effXbin = hEff.GetXaxis().FindBin("Sec%d" % sector)

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
                    if (abs(wheel) == 2 and "RB2out" in chName) or \
                       (abs(wheel) <= 1 and "RB2in" in chName):
                        suffixes = ["B", "M", "F"]# RB2in is divide by 3 rolls
                    else: suffixes = ["B", "F"]

                    nstrip = h.GetNbinsX()
                    nPerRoll = nstrip/len(suffixes)

                    for iroll, suffix in enumerate(suffixes):
                        rollName = chName+"_"+suffix

                        prefix = rollName.split('_')[1]
                        if prefix == "RB4-": prefix = "RB4,-"
                        elif prefix == "RB4": prefix = "RB4,-"
                        effYbin = hEff.GetYaxis().FindBin("%s_%s" % (prefix, suffix))
                        eff = hEff.GetBinContent(effXbin, effYbin)

                        ndead = 0.
                        for istrip in range(nPerRoll):
                            bin = 1+istrip+nPerRoll*iroll
                            if h.GetBinContent(bin) == 0: ndead += 1
                        fdead = ndead/nPerRoll
                        if fdead >= minDeadFrac:
                            #print "%s %f %f" % (rollName, fdead, eff)
                            if rollName not in blacklist: blacklist[rollName] = 1
                            else: blacklist[rollName] += 1

    for disk in range(-4, 5):
        if disk == 0: continue
        if disk > 0: dirDisk = "Endcap+/Disk_%d" % disk
        else: dirDisk = "Endcap-/Disk_%d" % disk
        for ring in range(2, 4):
            hEff = f.Get("%s/Efficiency_Roll_vs_Segment_Disk_%d" % (effFolderName, disk))
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

                        effXbin, effYbin = segment, 1+iroll+3*(ring-2)
                        eff = hEff.GetBinContent(effXbin, effYbin)

                        ndead = 0.
                        for istrip in range(nPerRoll):
                            bin = 1+istrip+nPerRoll*iroll
                            if h.GetBinContent(bin) == 0: ndead += 1
                        fdead = ndead/nPerRoll
                        if fdead > minDeadFrac:
                            #print "%s %f %f" % (rollName, fdead, eff)
                            if rollName not in blacklist: blacklist[rollName] = 1
                            else: blacklist[rollName] += 1

for rollName in sorted(blacklist.keys()):
    nDeadRun = blacklist[rollName]
    if 1.*nDeadRun/nRun > 0.5:
        print rollName, nDeadRun
