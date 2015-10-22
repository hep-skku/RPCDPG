#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TEfficiency.h"

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "RPCDPG/RecHitAnalysis/interface/RPCRecHitInfo.h"
#ifdef __MAKECINT__
#pragma link C++ class vector<RPCRefRecHitInfo>+;
#endif

using namespace std;

struct Plots {
  TH1F* hmuptA, * hmuetaA, * hmuphiA;
  TH2F* hrxyA;

  TH1F* hmuptP, * hmuetaP, * hmuphiP;
  TH2F* hrxyP;

  TH1F* hlx, * hdx;
  TH1F* hbx, * hcls;

  TDirectory* dir_;

  Plots() {
    dir_ = 0;
  };

  Plots(TDirectory* dir) {
    dir_ = dir;

    TString suffixA = "All", suffixP = "Pass";

    hmuptA  = new TH1F("hmupt"+suffixA, suffixA+";Muon p_{T} (GeV)", 100, 0, 100);
    hmuetaA = new TH1F("hmueta"+suffixA, suffixA+";Muon #eta", 100, -2.5, 2.5);
    hmuphiA = new TH1F("hmuphi"+suffixA, suffixA+";Muon #phi", 100, -3.15, 3.15);

    hmuptP  = new TH1F("hmupt"+suffixP, suffixP+";Muon p_{T} (GeV)", 100, 0, 100);
    hmuetaP = new TH1F("hmueta"+suffixP, suffixP+";Muon #eta", 100, -2.5, 2.5);
    hmuphiP = new TH1F("hmuphi"+suffixP, suffixP+";Muon #phi", 100, -3.15, 3.15);

    hrxyA = new TH2F("hrxy"+suffixA, suffixA+";Reference local x (cm);Reference local y (cm)", 200, -100, 100, 200, -100, 100);
    hrxyP = new TH2F("hrxy"+suffixP, suffixP+";Reference local x (cm);Reference local y (cm)", 200, -100, 100, 200, -100, 100);

    hlx = new TH1F("hlx", "Local x;Rechit local x (cm)", 200, -100, 100);
    hdx = new TH1F("hdx", "Local dx;Rechit local x - Refhit local x (cm)", 40, -20, 20);
    hbx = new TH1F("hbx", "BX;Bunch crossing", 10, -5, 5);
    hcls = new TH1F("hcls", "Cls;Cluster size", 10, 0, 10);

  }

  void fill(RPCRefRecHitInfo& info) {
    if ( !dir_ ) return;

    hmuptA->Fill(info.mupt);
    hmuetaA->Fill(info.mueta);
    hmuphiA->Fill(info.muphi);

    hrxyA->Fill(info.rlx, info.rly);

    if ( info.bx != -999 ) {
      hmuptP->Fill(info.mupt);
      hmuetaP->Fill(info.mueta);
      hmuphiP->Fill(info.muphi);

      hrxyP->Fill(info.rlx, info.rly);

      hlx->Fill(info.lx);
      hdx->Fill(info.dx);
      hbx->Fill(info.bx);
      hcls->Fill(info.clusterSize);
    }
  }
};

void analyzeRPCRefRecHit(TString fileName, TString outFileName = "result.root") {
  // Book histograms
  typedef std::vector<int> Key;
  std::map<Key, TH1F*> hAll, hPass;
  std::map<Key, Plots> hPlots;

  TFile* outFile = TFile::Open(outFileName, "RECREATE");
  if ( true ) {
    outFile->cd();
    TH1F* hRegionAll = new TH1F("hAll", "All", 4, -2, 2);
    TH1F* hRegionPass = new TH1F("hPass", "Pass", 4, -2, 2);
    hRegionAll->GetXaxis()->SetBinLabel(1, "All");
    hRegionPass->GetXaxis()->SetBinLabel(1, "Pass");
    hAll[Key()] = hRegionAll;
    hPass[Key()] = hRegionPass;
    hPlots[Key()] = Plots(outFile);
    for ( int region=-1; region<=1; ++region ) {
      const char* regionName;
      if ( region == 0 ) regionName = "Barrel";
      else if ( region == -1 ) regionName = "Endcap-";
      else regionName = "Endcap+";

      hRegionAll->GetXaxis()->SetBinLabel(region+3, regionName);
      hRegionPass->GetXaxis()->SetBinLabel(region+3, regionName);

      outFile->mkdir(regionName);
      TDirectory* regionDir = outFile->GetDirectory(regionName);
      regionDir->cd();
      hPlots[Key({region})] = Plots(regionDir);

      if ( region == 0 ) {
        TH1F* hWheelAll  = new TH1F("hAll", "All;Wheel", 5, -2.5, 2.5);
        TH1F* hWheelPass = new TH1F("hPass", "Pass;Wheel", 5, -2.5, 2.5);

        hAll[Key({region})] = hWheelAll;
        hPass[Key({region})] = hWheelPass;

        for ( int wheel=-2; wheel<=2; ++wheel ) {
          regionDir->mkdir(Form("Wheel%+d", wheel));
          TDirectory* wheelDir = regionDir->GetDirectory(Form("Wheel%+d", wheel));
          wheelDir->cd();

          TH1F* hStationAll  = new TH1F("hAll", "All;Station",4, 0.5, 4.5);
          TH1F* hStationPass = new TH1F("hPass", "Pass;Station",4, 0.5, 4.5);

          hAll[Key({region,wheel})] = hStationAll;
          hPass[Key({region,wheel})] = hStationPass;
          hPlots[Key({region,wheel})] = Plots(wheelDir);

          for ( int station=1; station<=4; ++station ) {
            wheelDir->mkdir(Form("Station%+d",station));
            TDirectory* stationDir = wheelDir->GetDirectory(Form("Station%+d",station));
            stationDir->cd();

            TH1F* hSectorAll  = new TH1F("hAll","All;Sector", 12, 0.5, 12.5);
            TH1F* hSectorPass = new TH1F("hPass","Pass;Sector", 12, 0.5, 12.5);

            hAll[Key({region,wheel,station})] = hSectorAll;
            hPass[Key({region,wheel,station})] = hSectorPass;
            hPlots[Key({region,wheel,station})] = Plots(stationDir);

            for ( int sector=1; sector<=12; ++sector ) {
              stationDir->mkdir(Form("Sector%+d",sector));
              TDirectory* sectorDir = stationDir->GetDirectory(Form("Sector%+d",sector));
              sectorDir->cd();

              TH1F* hLayerAll  = new TH1F("hAll","All;Layer", 2, 0.5, 2.5);
              TH1F* hLayerPass = new TH1F("hPass","Pass;Layer", 2, 0.5, 2.5);

              hAll[Key({region,wheel,station,sector})] = hLayerAll;
              hPass[Key({region,wheel,station,sector})] = hLayerPass;
              hPlots[Key({region,wheel,station,sector})] = Plots(sectorDir);

              for ( int layer=1; layer<=2; ++layer ) {
                sectorDir->mkdir(Form("Layer%+d",layer));
                TDirectory* layerDir = sectorDir->GetDirectory(Form("Layer%+d",layer));
                layerDir->cd();

                TH1F* hRollAll  = new TH1F("hAll","All;Roll", 3, 0.5, 3.5);
                TH1F* hRollPass = new TH1F("hPass","Pass;Roll", 3, 0.5, 3.5);

                hAll[Key({region,wheel,station,sector,layer})] = hRollAll;
                hPass[Key({region,wheel,station,sector,layer})] = hRollPass;
                hPlots[Key({region,wheel,station,sector,layer})] = Plots(layerDir);

/*                for ( int roll=1; roll<=4; ++roll ) {
                  layerDir->mkdir(Form("Roll%d", roll));
                  TDirectory* rollDir = layerDir->GetDirectory(Form("Roll%d", roll));
                  hPlots[Key({region,wheel,station,sector,layer,roll})] = Plots(rollDir);
                }*/
              }
            }
          }
        }
      }
      else {
        const double xmin = region < 0 ? -4.5 : -0.5;
        const double xmax = region < 0 ? 0.5 : 4.5;
        TH1F* hDiskAll  = new TH1F("hAll" , "All;Disk", 4, 0.5, 4.5);
        TH1F* hDiskPass = new TH1F("hPass", "All;Disk", 4, 0.5, 4.5);
        hAll[Key({region})] = hDiskAll;
        hPass[Key({region})] = hDiskPass;

        for ( int disk=1; disk<=4; ++disk ) {
          // For endcap - "C++ ring" is "Disk" (how apart from the barrel)
          regionDir->mkdir(Form("Disk%+d", region*disk));
          TDirectory* diskDir = regionDir->GetDirectory(Form("Disk%+d", region*disk));
          diskDir->cd();

          TH1F* hSectorAll  = new TH1F("hAll", "All;Sector", 12, 0.5, 12.5);
          TH1F* hSectorPass = new TH1F("hPass", "Pass;Sector", 12, 0.5, 12.5);

          hAll[Key({region,disk})] = hSectorAll;
          hPass[Key({region,disk})] = hSectorPass;
          hPlots[Key({region,disk})] = Plots(diskDir);

          for ( int sector=1; sector<=12; ++sector ) {
            diskDir->mkdir(Form("Sector%+d",sector));
            TDirectory* sectorDir = diskDir->GetDirectory(Form("Sector%+d",sector));
            sectorDir->cd();

            TH1F* hStationAll  = new TH1F("hAll","All;Station", 4, 0.5, 4.5);
            TH1F* hStationPass = new TH1F("hPass","Pass;Station", 4, 0.5, 4.5);

            hAll[Key({region,disk,sector})] = hStationAll;
            hPass[Key({region,disk,sector})] = hStationPass;
            hPlots[Key({region,disk,sector})] = Plots(sectorDir);
            for ( int station=1; station<=4; ++station ) {
              // For endcap - "C++ sector" is "Ring" (how far from beamline)
              sectorDir->mkdir(Form("Station%+d",station));
              TDirectory* stationDir = sectorDir->GetDirectory(Form("Station%+d",station));
              stationDir->cd();

              TH1F* hLayerAll  = new TH1F("hAll","All;Layer", 2, 0.5, 2.5);
              TH1F* hLayerPass = new TH1F("hPass","Pass;Layer", 2, 0.5, 2.5);

              hAll[Key({region,disk,sector,station})] = hLayerAll;
              hPass[Key({region,disk,sector,station})] = hLayerPass;
              hPlots[Key({region,disk,sector,station})] = Plots(stationDir);

              for ( int layer=1; layer<=2; ++layer ) {
                stationDir->mkdir(Form("Layer%+d",layer));
                TDirectory* layerDir = stationDir->GetDirectory(Form("Layer%+d",layer));
                layerDir->cd();

                TH1F* hRollAll  = new TH1F("hAll","All;Roll", 3, 0.5, 3.5);
                TH1F* hRollPass = new TH1F("hPass","Pass;Roll", 3, 0.5, 3.5);

                hAll[Key({region,disk,sector,station,layer})] = hRollAll;
                hPass[Key({region,disk,sector,station,layer})] = hRollPass;
                hPlots[Key({region,disk,sector,station,layer})] = Plots(layerDir);

/*                for ( int roll=1; roll<=3; ++roll ) {
                  layerDir->mkdir(Form("Roll%c", roll+'A'));
                  TDirectory* rollDir = layerDir->GetDirectory(Form("Roll%c", roll+'A'));
                  hPlots[Key({region,disk,sector,station,layer,roll})] = Plots(rollDir);
                }*/
              }
            }
          }
        }
      }
    }
  }

  // Analyze input ntuple
  TFile* f = TFile::Open(fileName);
  TTree* tree1 = (TTree*)f->Get("trackRPC/tree");

  int run, lumi, event;
  tree1->SetBranchAddress("run"  , &run  );
  tree1->SetBranchAddress("lumi" , &lumi );
  tree1->SetBranchAddress("event", &event);

  std::vector<RPCRefRecHitInfo>* recHits1 = new std::vector<RPCRefRecHitInfo>;
  tree1->SetBranchAddress("rpcInfo", &recHits1);

  const int nEntry1 = tree1->GetEntries();
  cout << "Getting events : " << nEntry1 << endl;

  std::set<int> runs;
  for ( int iEntry1 = 0; iEntry1 < nEntry1; ++iEntry1 ) {
    const double permil = 1000.*iEntry1/nEntry1;

    tree1->GetEntry(iEntry1);
    cout << Form("Processed %.1f%%, % 9d/%09d, run=%07d\r", permil/10., iEntry1, nEntry1, run);

    runs.insert(run);

    const int nRecHit = recHits1->size();
    for ( int iRecHit = 0; iRecHit < nRecHit; ++iRecHit ) {
      RPCRefRecHitInfo& recHit = recHits1->at(iRecHit);

      // Apply pT cuts
      //if ( recHit.mupt < 20 || recHit.mupt > 100 ) continue;

      hAll[Key()]->Fill(recHit.region);
      hAll[Key({recHit.region})]->Fill(recHit.ring);
      hPlots[Key()].fill(recHit);
      hPlots[Key({recHit.region})].fill(recHit);
      if ( recHit.region == 0 ) {
        // For barrel - "C++ ring" is "Wheel"
        hAll[Key({recHit.region, recHit.ring})]->Fill(recHit.station);
        hAll[Key({recHit.region, recHit.ring, recHit.station})]->Fill(recHit.sector);
        hAll[Key({recHit.region, recHit.ring, recHit.station, recHit.sector})]->Fill(recHit.layer);
        hAll[Key({recHit.region, recHit.ring, recHit.station, recHit.sector, recHit.layer})]->Fill(recHit.roll);

        hPlots[Key({recHit.region, recHit.ring})].fill(recHit);
        hPlots[Key({recHit.region, recHit.ring, recHit.station})].fill(recHit);
        hPlots[Key({recHit.region, recHit.ring, recHit.station, recHit.sector})].fill(recHit);
        hPlots[Key({recHit.region, recHit.ring, recHit.station, recHit.sector, recHit.layer})].fill(recHit);
        //hPlots[Key({recHit.region, recHit.ring, recHit.station, recHit.sector, recHit.layer, recHit.roll})].fill(recHit);
      }
      else {
        // For endcap - "C++ ring" is "Disk" (how apart from the barrel)
        // For endcap - "C++ sector" is "Ring" (how far from beamline)
        hAll[Key({recHit.region, recHit.ring})]->Fill(recHit.sector);
        hAll[Key({recHit.region, recHit.ring, recHit.sector})]->Fill(recHit.station);
        hAll[Key({recHit.region, recHit.ring, recHit.sector, recHit.station})]->Fill(recHit.layer);
        hAll[Key({recHit.region, recHit.ring, recHit.sector, recHit.station, recHit.layer})]->Fill(recHit.roll);

        hPlots[Key({recHit.region, recHit.ring})].fill(recHit);
        hPlots[Key({recHit.region, recHit.ring, recHit.sector})].fill(recHit);
        hPlots[Key({recHit.region, recHit.ring, recHit.sector, recHit.station})].fill(recHit);
        hPlots[Key({recHit.region, recHit.ring, recHit.sector, recHit.station, recHit.layer})].fill(recHit);
        //hPlots[Key({recHit.region, recHit.ring, recHit.sector, recHit.station, recHit.layer, recHit.roll})].fill(recHit);
      }

      if ( recHit.bx != -999 ) {
        hPass[Key()]->Fill(recHit.region);
        hPass[Key({recHit.region})]->Fill(recHit.ring);
        if ( recHit.region == 0 ) {
          hPass[Key({recHit.region, recHit.ring})]->Fill(recHit.station);
          hPass[Key({recHit.region, recHit.ring, recHit.station})]->Fill(recHit.sector);
          hPass[Key({recHit.region, recHit.ring, recHit.station, recHit.sector})]->Fill(recHit.layer);
          hPass[Key({recHit.region, recHit.ring, recHit.station, recHit.sector, recHit.layer})]->Fill(recHit.roll);
        }
        else {
          hPass[Key({recHit.region, recHit.ring})]->Fill(recHit.sector);
          hPass[Key({recHit.region, recHit.ring, recHit.sector})]->Fill(recHit.station);
          hPass[Key({recHit.region, recHit.ring, recHit.sector, recHit.station})]->Fill(recHit.layer);
          hPass[Key({recHit.region, recHit.ring, recHit.sector, recHit.station, recHit.layer})]->Fill(recHit.roll);
        } 
      }
    }
  }
  // Set the first bin of the very first histogram to count "all statistics"
  hAll[Key()]->SetBinContent(1, hAll[Key()]->Integral());
  hPass[Key()]->SetBinContent(1, hPass[Key()]->Integral());

  outFile->cd();
  TH1F* hEffBarrel  = new TH1F("hEffBarrel" , "Barrel;Efficiency (\%)", 102, -1, 101);
  TH1F* hEffEndcapM = new TH1F("hEffEndcapM", "Endcap-;Efficiency (\%)", 102, -1, 101);
  TH1F* hEffEndcapP = new TH1F("hEffEndcapP", "Endcap+;Efficiency (\%)", 102, -1, 101);
  std::map<Key, TEfficiency*> effs;
  for ( auto hh = hPass.begin(); hh != hPass.end(); ++hh ) {
    auto& key = hh->first;
    auto& hhPass = hh->second;
    auto& hhAll  = hAll[key];

    // Set the output base directory
    hhPass->GetDirectory()->cd();

    auto eff = new TEfficiency(*hhPass, *hhAll);
    eff->SetName(Form("%s_eff", hhAll->GetName()));
    eff->SetDirectory(gDirectory);
    effs[key] = eff;

    if ( key.size() == 5 && hhAll->GetEntries() > 100 ) {
      for ( int i=1; i<=hhPass->GetNbinsX(); ++i ) {
        const double den = hhAll->GetBinContent(i);
        const double num = hhPass->GetBinContent(i);
        if ( den < 100 ) continue;
        const double eff = num/den*100;
        if      ( key[0] == 0  ) hEffBarrel->Fill(eff);
        else if ( key[0] == -1 ) hEffEndcapM->Fill(eff);
        else if ( key[0] == +1 ) hEffEndcapP->Fill(eff);
      }
    }
  }

  outFile->Write();

  cout << endl;
  cout << "======================================================\n";          
  cout << " Analyzed " << runs.size() << " runs, " << nEntry1 << " events : \n";
  cout << " Overall efficiency = " << effs[Key()]->GetEfficiency(1)*100 << "\%\n";
  cout << "======================================================\n";          

  delete recHits1;

}
