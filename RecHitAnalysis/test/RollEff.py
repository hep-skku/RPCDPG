from ROOT import *

output = TFile.Open("result.root","RECREATE")
nTotal,nRegion, nRing, nStation =1, 1, 1, 1
nSector, nLayer, nSubsector, nRoll = 1, 2, 6, 3

nTotRoll = nRegion*nRing*nStation*nSector*nLayer*nSubsector*nRoll
rootFile = "/afs/cern.ch/user/j/jhgoh/public/RPC/20151019_RPCEffic/ntuple.root"
#rootFile = "ntuple.root"
ntuple = TFile.Open(rootFile)
rpc = ntuple.Get("trackRPC/tree")

hEffVsTotal = TH1F("hEffVsTotal","Efficiency vs. Total;Total;Efficiency",nTotal,0,1)
hEffVsRegion = TH1F("hEffVsRegion","Efficiency vs. Region;Region;Efficiency",nRegion,-1,2)
hEffVsRing = TH1F("hEffVsRing","Efficiency vs. Ring;Ring;Efficiency",nRing,-2,4)
hEffVsStation = TH1F("hEffVsStation","Efficiency vs. Station;Station;Efficiency",nStation,1,5)
hEffVsSector = TH1F("hEffVsSector","Efficiency vs. Sector;Sector;Efficiency",nSector,1,13)
hEffVsLayer = TH1F("hEffVsLayer","Efficiency vs. Layer;Layer;Efficiency",nLayer,1,3)
hEffVsSubsector = TH1F("hEffVsSubsector","Efficiency vs. Subsector;Subsector;Efficiency",nSubsector,1,7)
hEffVsRoll = TH1F("hEffVsRoll","Efficiency vs. Roll;Roll;Efficiency",nRoll,1,4)

def Eff(h,i,cut):
  hBxRecHit = TH1F("hBxRecHit","",10,-1000,50)
  hBx = TH1F("hBx","",10,-1000,100)

  rpc.Project("hBxRecHit","rpcInfo.bx","rpcInfo.bx!=-999 &&"+cut)
  rpc.Project("hBx","rpcInfo.bx","1 && "+cut)
  if hBx.Integral() != 0:
    eff = hBxRecHit.Integral()/hBx.Integral()
#    print eff
    h.SetBinContent(i+1,eff)
  hBxRecHit.Reset()
  hBx.Reset()

for s0 in range(nTotal):
  k = 0
  for s1 in range(nRegion):
    for s2 in range(nRing):
      for s3 in range(nStation):
        for s4 in range(nSector):
          for s5 in range(nLayer):
            for s6 in range(nSubsector):
              output.mkdir("RPC/Region%d/Ring%d/Station%d/Sector%d/Layer%d/Subsector%d"%(s1-1,s2-2,s3+1,s4+1,s5+1,s6+1))
              for s7 in range(nRoll):
                cut = "rpcInfo.region==%d && rpcInfo.ring==%d && rpcInfo.station==%d && rpcInfo.sector==%d && rpcInfo.layer==%d && rpcInfo.subsector==%d && rpcInfo.roll==%d"%(s1-1,s2-2,s3+1,s4+1,s5+1,s6+1,s7+1)
                Eff(hEffVsRoll,s7,cut)
                step = s1*nRing*nStation*nSector*nLayer*nSubsector*nRoll + s2*nStation*nSector*nLayer*nSubsector*nRoll + s3*nSector*nLayer*nSubsector*nRoll + s4*nLayer*nSubsector*nRoll + s5*nSubsector*nRoll + s6*nRoll + s7
                if int(100./nTotRoll*step)//10!=k:
                  k = int(100./nTotRoll*step)//10
                  print "%d"%(k*10), "% complited" 
              cut = "rpcInfo.region==%d && rpcInfo.ring==%d && rpcInfo.station==%d && rpcInfo.sector==%d && rpcInfo.layer==%d && rpcInfo.subsector==%d"%(s1-1,s2-2,s3+1,s4+1,s5+1,s6+1)
              Eff(hEffVsSubsector,s6,cut)
              output.cd("RPC/Region%d/Ring%d/Station%d/Sector%d/Layer%d/Subsector%d"%(s1-1,s2-2,s3+1,s4+1,s5+1,s6+1))
              hEffVsRoll.Write()
              hEffVsRoll.Reset()
            cut = "rpcInfo.region==%d && rpcInfo.ring==%d && rpcInfo.station==%d && rpcInfo.sector==%d && rpcInfo.layer==%d"%(s1-1,s2-2,s3+1,s4+1,s5+1)
            Eff(hEffVsLayer,s5,cut)
            output.cd("RPC/Region%d/Ring%d/Station%d/Sector%d/Layer%d/"%(s1-1,s2-2,s3+1,s4+1,s5+1))
            hEffVsSubsector.Write()
            hEffVsSubsector.Reset()
          cut = "rpcInfo.region==%d && rpcInfo.ring==%d && rpcInfo.station==%d && rpcInfo.sector==%d"%(s1-1,s2-2,s3+1,s4+1)
          Eff(hEffVsSector,s4,cut)
          output.cd("RPC/Region%d/Ring%d/Station%d/Sector%d/"%(s1-1,s2-2,s3+1,s4+1))
          hEffVsLayer.Write()
          hEffVsLayer.Reset()
        cut = "rpcInfo.region==%d && rpcInfo.ring==%d && rpcInfo.station==%d"%(s1-1,s2-2,s3+1)
        Eff(hEffVsStation,s3,cut)
        output.cd("RPC/Region%d/Ring%d/Station%d/"%(s1-1,s2-2,s3+1))
        hEffVsSector.Write()
        hEffVsSector.Reset()
      cut = "rpcInfo.region==%d && rpcInfo.ring==%d"%(s1-1,s2-2)
      Eff(hEffVsRing,s2,cut)
      output.cd("RPC/Region%d/Ring%d/"%(s1-1,s2-2))
      hEffVsStation.Write()
      hEffVsStation.Reset()
    cut = "rpcInfo.region==%d"%(s1-1)
    Eff(hEffVsRegion,s1,cut)
    output.cd("RPC/Region%d/"%(s1-1))
    hEffVsRing.Write()
    hEffVsRing.Reset()
  cut = "1"
  Eff(hEffVsTotal,s0,cut)
  output.cd("RPC")
  hEffVsRegion.Write()
  hEffVsRegion.Reset()
output.cd("")
hEffVsTotal.Write()          
output.Write()
output.Close()
