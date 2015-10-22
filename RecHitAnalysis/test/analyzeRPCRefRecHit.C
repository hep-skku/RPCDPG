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

void analyzeRPCRefRecHit(TString fileName, TString outFileName = "result.root") {
  // Book histograms
  typedef std::vector<int> Key;
  std::map<Key, TH1F*> hAll, hPass;

  TFile* outFile = TFile::Open(outFileName, "RECREATE");
  if ( true ) {
    outFile->cd();
    TH1F* hRegionAll = new TH1F("hRegionAll", "All", 4, -2, 2);
    TH1F* hRegionPass = new TH1F("hRegionPass", "Pass", 4, -2, 2);
    hRegionAll->GetXaxis()->SetBinLabel(1, "All");
    hRegionPass->GetXaxis()->SetBinLabel(1, "Pass");
    hAll[Key()] = hRegionAll;
    hPass[Key()] = hRegionPass;
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

      if ( region == 0 ) {
        TH1F* hWheelAll  = new TH1F("hWheelAll", "All;Wheel", 5, -2.5, 2.5);
        TH1F* hWheelPass = new TH1F("hWheelPass", "Pass;Wheel", 5, -2.5, 2.5);

        hAll[Key({region})] = hWheelAll;
        hPass[Key({region})] = hWheelPass;

        for ( int wheel=-2; wheel<=2; ++wheel ) {
          regionDir->mkdir(Form("Wheel%+d", wheel));
          TDirectory* wheelDir = regionDir->GetDirectory(Form("Wheel%+d", wheel));
          wheelDir->cd();

          TH1F* hStationAll = new TH1F("hStationAll", "All;Station",4, 0.5, 4.5);
          TH1F* hStationPass = new TH1F("hStationPass", "Pass;Station",4, 0.5, 4.5);

          hAll[Key({region,wheel})] = hStationAll;
          hPass[Key({region,wheel})] = hStationPass;


          for ( int station=1; station<=4; ++station ) {
            wheelDir->mkdir(Form("Station%+d",station));
            TDirectory* stationDir = wheelDir->GetDirectory(Form("Station%+d",station));
            stationDir->cd();

            TH1F* hSectorAll = new TH1F("hSectorAll","All;Sector", 12, 0.5, 12.5);
            TH1F* hSectorPass = new TH1F("hSectorPass","Pass;Sector", 12, 0.5, 12.5);

            hAll[Key({region,wheel,station})] = hSectorAll;
            hPass[Key({region,wheel,station})] = hSectorPass;

            for ( int sector=1; sector<=12; ++sector ) {
              stationDir->mkdir(Form("Sector%+d",sector));
              TDirectory* sectorDir = stationDir->GetDirectory(Form("Sector%+d",sector));
              sectorDir->cd();

              TH1F* hLayerAll = new TH1F("hLayerAll","All;Layer", 2, 0.5, 2.5);
              TH1F* hLayerPass = new TH1F("hLayerPass","Pass;Layer", 2, 0.5, 2.5);

              hAll[Key({region,wheel,station,sector})] = hLayerAll;
              hPass[Key({region,wheel,station,sector})] = hLayerPass;

              for ( int layer=1; layer<=2; ++layer ) {
                sectorDir->mkdir(Form("Layer%+d",layer));
                TDirectory* layerDir = sectorDir->GetDirectory(Form("Layer%+d",layer));
                layerDir->cd();

                TH1F* hRollAll = new TH1F("hRollAll","All;Roll", 3, 0.5, 3.5);
                TH1F* hRollPass = new TH1F("hRollPass","Pass;Roll", 3, 0.5, 3.5);

                hAll[Key({region,wheel,station,sector,layer})] = hRollAll;
                hPass[Key({region,wheel,station,sector,layer})] = hRollPass;

              }
            }
          }
        }
      }
      else {
        const double xmin = region < 0 ? -4.5 : -0.5;
        const double xmax = region < 0 ? 0.5 : 4.5;
        TH1F* hDiskAll = new TH1F(region < 0 ? "hDisk-All" : "hDisk+All", "All;Disk", 4, 0.5, 4.5);
        TH1F* hDiskPass = new TH1F(region < 0 ? "hDisk-Pass" : "hDisk+Pass", "Pass;Disk", 4, 0.5, 4.5);
        hAll[Key({region})] = hDiskAll;
        hPass[Key({region})] = hDiskPass;

        for ( int disk=1; disk<=4; ++disk ) {
          // For endcap - "C++ ring" is "Disk" (how apart from the barrel)
          regionDir->mkdir(Form("Disk%+d", region*disk));
          TDirectory* diskDir = regionDir->GetDirectory(Form("Disk%+d", region*disk));
          diskDir->cd();

          TH1F* hSectorAll = new TH1F("hSectorAll", "All;Sector", 12, 0.5, 12.5);
          TH1F* hSectorPass = new TH1F("hSectorPass", "Pass;Sector", 12, 0.5, 12.5);

          hAll[Key({region,disk})] = hSectorAll;
          hPass[Key({region,disk})] = hSectorPass;

          for ( int sector=1; sector<=12; ++sector ) {
            diskDir->mkdir(Form("Sector%+d",sector));
            TDirectory* sectorDir = diskDir->GetDirectory(Form("Sector%+d",sector));
            sectorDir->cd();

            TH1F* hStationAll = new TH1F("hStationAll","All;Station", 4, 0.5, 4.5);
            TH1F* hStationPass = new TH1F("hStationPass","Pass;Station", 4, 0.5, 4.5);

            hAll[Key({region,disk,sector})] = hStationAll;
            hPass[Key({region,disk,sector})] = hStationPass;
            for ( int station=1; station<=4; ++station ) {
              // For endcap - "C++ sector" is "Ring" (how far from beamline)
              sectorDir->mkdir(Form("Station%+d",station));
              TDirectory* stationDir = sectorDir->GetDirectory(Form("Station%+d",station));
              stationDir->cd();

              TH1F* hLayerAll = new TH1F("hLayerAll","All;Layer", 2, 0.5, 2.5);
              TH1F* hLayerPass = new TH1F("hLayerPass","Pass;Layer", 2, 0.5, 2.5);

              hAll[Key({region,disk,sector,station})] = hLayerAll;
              hPass[Key({region,disk,sector,station})] = hLayerPass;

              for ( int layer=1; layer<=2; ++layer ) {
                stationDir->mkdir(Form("Layer%+d",layer));
                TDirectory* layerDir = stationDir->GetDirectory(Form("Layer%+d",layer));
                layerDir->cd();

                TH1F* hRollAll = new TH1F("hRollAll","All;Roll", 3, 0.5, 3.5);
                TH1F* hRollPass = new TH1F("hRollPass","Pass;Roll", 3, 0.5, 3.5);

                hAll[Key({region,disk,sector,station,layer})] = hRollAll;
                hPass[Key({region,disk,sector,station,layer})] = hRollPass;

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

      hAll[Key()]->Fill(recHit.region);
      hAll[Key({recHit.region})]->Fill(recHit.ring);
      if ( recHit.region == 0 ) {
        // For barrel - "C++ ring" is "Wheel"
        hAll[Key({recHit.region, recHit.ring})]->Fill(recHit.station);
        hAll[Key({recHit.region, recHit.ring, recHit.station})]->Fill(recHit.sector);
        hAll[Key({recHit.region, recHit.ring, recHit.station, recHit.sector})]->Fill(recHit.layer);
        hAll[Key({recHit.region, recHit.ring, recHit.station, recHit.sector, recHit.layer})]->Fill(recHit.roll);
      }
      else {
        // For endcap - "C++ ring" is "Disk" (how apart from the barrel)
        // For endcap - "C++ sector" is "Ring" (how far from beamline)
        hAll[Key({recHit.region, recHit.ring})]->Fill(recHit.sector);
        hAll[Key({recHit.region, recHit.ring, recHit.sector})]->Fill(recHit.station);
        hAll[Key({recHit.region, recHit.ring, recHit.sector, recHit.station})]->Fill(recHit.layer);
        hAll[Key({recHit.region, recHit.ring, recHit.sector, recHit.station, recHit.layer})]->Fill(recHit.roll);
      }

      if ( recHit.dx != -999 ) {
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

  std::vector<TEfficiency*> effs;
  for ( auto hh = hPass.begin(); hh != hPass.end(); ++hh ) {
    auto key = hh->first;
    auto hhPass = hh->second;
    auto hhAll  = hAll[key];

    // Set the output base directory
    hhPass->GetDirectory()->cd();

    effs.push_back(new TEfficiency(*hhPass, *hhAll));
    effs.back()->SetName(Form("%s_eff", hhAll->GetName()));
    effs.back()->SetDirectory(gDirectory);
  }

  outFile->Write();

  cout << endl;
  cout << "======================================================\n";          
  cout << " Analyzed " << runs.size() << " runs, " << nEntry1 << " events : \n";
  cout << "======================================================\n";          

  delete recHits1;

}
