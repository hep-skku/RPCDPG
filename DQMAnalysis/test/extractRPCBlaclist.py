#!/usr/bin/env python

import sys, os
from ROOT import *

minEvents = 10000
minDeadFrac = 0.8

#basePath = "/store1/chanwook/data"
basePath = "/store1/jhgoh/DQM/SingleMu/Commissioning2015-PromptReco-v1/"
#basePath = "/store1/jhgoh/RPC/DQM/SingleMu2012"
#basePath = "/store1/jhgoh/RPC/DQM/Cosmics_2015"

nRoll = 0
nRun = 0
rollsByRun = {}

hRateBarrel = TH1F("hRateBarrel", "Barrel;Strip occupancy per event", 1001, -1e-5, 1e-2)
hRateEndcap = TH1F("hRateEndcap", "Endcap;Strip occupancy per event", 1001, -1e-5, 1e-2)

for fName in sorted(os.listdir(basePath)):
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

    nLSRPC = 0
    hHVStatus = f.Get("DQMData/Run %d/RPC/Run summary/DCSInfo/rpcHVStatus" % runNumber)
    for bin in range(1, hHVStatus.GetNbinsX()+1):
        hvBit = hHVStatus.GetBinContent(bin, 1)
        if hvBit >=  1-1e-5: nLSRPC += 1

    print runNumber
    rollToFdead = {}

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

                        ndead, nhit = 0., 0
                        for istrip in range(nPerRoll):
                            bin = 1+istrip+nPerRoll*iroll
                            occup = h.GetBinContent(bin)
                            nhit += occup
                            if occup == 0:
                                ndead += 1
                                hRateBarrel.Fill(-1e-6)
                            else:
                                rate = occup/nRPCEvents
                                hRateBarrel.Fill(rate)

                        fdead = ndead/nPerRoll
                        rollToFdead[rollName] = (fdead, nhit, eff)

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

                        ndead, nhit  = 0., 0
                        for istrip in range(nPerRoll):
                            bin = 1+istrip+nPerRoll*iroll
                            occup = h.GetBinContent(bin)
                            nhit += occup
                            if occup == 0:
                                ndead += 1
                                hRateEndcap.Fill(-1e-6)
                            else:
                                rate = occup/nRPCEvents
                                hRateEndcap.Fill(rate)

                        fdead = ndead/nPerRoll
                        rollToFdead[rollName] = (fdead, nhit, eff)

    nRun += 1
    nRoll = max(nRoll, len(rollToFdead))
    rollsByRun[runNumber] = (rollToFdead, nLSRPC, nRPCEvents)

hFdeadVsEff = TH2F("fdeadVsEff", "fdead vs efficiency;1-(Dead fraction) [%];Efficiency [%]", 102, -1, 101, 103, -2, 101)
hFdeadHistory = TH2F("fdeadHistory", "fdeadHistory", nRun, 0, nRun, nRoll, 0, nRoll)
hNhitVsEff = TH2F("hNhitVsEff", "nhit vs efficiency;RPC hit multiplicity;Efficiency [%]", 50, 0, 1000, 103, -2, 101)
for irun, run in enumerate(sorted(rollsByRun.keys())):
    rollToFdead, nLS, nRPCEvents = rollsByRun[run]

    outFdead = open("fDead_run%d.txt" % run, "w")
    outBlist = open("Blacklist_run%d.txt" % run, "w")
    print>>outFdead, "# Run = %d" % run
    print>>outFdead, "# nLS with RPC on = %d" % nLS
    print>>outFdead, "# nEvents with RPC hit = %d" % nRPCEvents
    print>>outFdead, "# Roll_Name\tnDeadStrip/nStrip\tEfficiency"

    print>>outBlist, "# Run = %d" % run
    print>>outBlist, "# nLS with RPC on = %d" % nLS
    print>>outBlist, "# nEvents with RPC hit = %d" % nRPCEvents
    print>>outBlist, "# Roll_Name\tnDeadStrip/nStrip\tEfficiency"

    for iroll, roll in enumerate(sorted(rollToFdead.keys())):
        (fdead, nhit, eff) = rollToFdead[roll]
        if eff == 1.0: eff += 1e-9
        if eff <= -0.9: eff = -2
        if eff == 0.0: eff -= 1e-9
        print>>outFdead, "%s\t%f\t%f\t%f" % (roll, fdead, nhit, eff)
        if fdead >= 0.8: print>>outBlist, "%s\t%f\t%f\t%f" % (roll, fdead, nhit, eff) 

        hFdeadHistory.SetBinContent(irun+1, iroll+1, fdead)
        hFdeadVsEff.Fill((1-fdead)*100, eff)
        hNhitVsEff.Fill(min(999, nhit), eff)

    outFdead.close()
    outBlist.close()

cRate = TCanvas("cRate", "cRate", 500, 500)
hRateBarrel.SetLineColor(kRed)
hRateEndcap.SetLineColor(kBlue)
hRateBarrel.Draw()
hRateEndcap.Draw("sames")

cFHistory = TCanvas("cFHistory", "cFHistory", 500, 500)
hFdeadHistory.Draw("COLZ")

cFCorr = TCanvas("cFCorr", "cFCorr", 500, 500)
hFdeadVsEff.Draw("COLZ")

cNCorr = TCanvas("cNCorr", "cNCorr", 500, 500)
hNhitVsEff.Draw("COLZ")
