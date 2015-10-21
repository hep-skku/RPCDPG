#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

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
    hRegionPass->GetXaxis()->SetBinLabel(1, "All");
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

          for ( int station=1; station<=4; ++station ) {
            for ( int sector=1; sector<=12; ++sector ) {
              for ( int layer=1; layer<=2; ++layer ) {
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

          for ( int sector=1; sector<=12; ++sector ) {
            for ( int station=1; station<=4; ++station ) {
              // For endcap - "C++ sector" is "Ring" (how far from beamline)
              for ( int layer=1; layer<=2; ++layer ) {
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
        //hAll[Key({recHit.region, recHit.ring})]->Fill(recHit.station);
        //hAll[Key({recHit.region, recHit.ring, recHit.station})]->Fill(recHit.sector);
      }
      else {
        // For endcap - "C++ ring" is "Disk" (how apart from the barrel)
        // For endcap - "C++ sector" is "Ring" (how far from beamline)
        //hAll[Key({recHit.region, recHit.ring})]->Fill(recHit.station);
      }

      if ( recHit.dx != -999 ) {
        hPass[Key()]->Fill(recHit.region);
        hPass[Key({recHit.region})]->Fill(recHit.ring);
      }
    }
  }

  hAll[Key()]->SetBinContent(1, hAll[Key()]->Integral());
  hPass[Key()]->SetBinContent(1, hPass[Key()]->Integral());
  outFile->Write();

  cout << endl;
  cout << "======================================================\n";          
  cout << " Analyzed " << runs.size() << " runs, " << nEntry1 << " events : \n";
  cout << "======================================================\n";          

  delete recHits1;

}
