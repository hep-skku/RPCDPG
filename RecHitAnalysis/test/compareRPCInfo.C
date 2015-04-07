#include "TString.h"

void compareRPCInfo(TString fileName1, TString fileName2);

void compareRPCInfo()
{
  TString baseDir = "./";

/*
  compareRPCInfo(baseDir+"rpcVal_430pre2.root", baseDir+"rpcVal_430pre4.root");
  compareRPCInfo(baseDir+"rpcVal_430pre4.root", baseDir+"rpcVal_430pre5.root");
  compareRPCInfo(baseDir+"rpcVal_430pre5.root", baseDir+"rpcVal_430pre6.root");
  compareRPCInfo(baseDir+"rpcVal_430pre6.root", baseDir+"rpcVal_430pre7.root");

  compareRPCInfo(baseDir+"rpcVal_430pre7.root", baseDir+"rpcVal_440pre1.root");
  compareRPCInfo(baseDir+"rpcVal_440pre1.root", baseDir+"rpcVal_440pre2.root");
  compareRPCInfo(baseDir+"rpcVal_440pre2.root", baseDir+"rpcVal_440pre3.root");
  compareRPCInfo(baseDir+"rpcVal_440pre3.root", baseDir+"rpcVal_440pre4.root");
  compareRPCInfo(baseDir+"rpcVal_440pre4.root", baseDir+"rpcVal_440pre5.root");
  compareRPCInfo(baseDir+"rpcVal_440pre5.root", baseDir+"rpcVal_440pre7.root");
  compareRPCInfo(baseDir+"rpcVal_440pre7.root", baseDir+"rpcVal_440pre8.root");
  compareRPCInfo(baseDir+"rpcVal_440pre8.root", baseDir+"rpcVal_440pre9.root");
  compareRPCInfo(baseDir+"rpcVal_440pre9.root", baseDir+"rpcVal_440pre10.root");
  compareRPCInfo(baseDir+"rpcVal_440pre10.root", baseDir+"rpcVal_440.root");

  compareRPCInfo(baseDir+"rpcVal_440.root", baseDir+"rpcVal_500pre2.root");
  compareRPCInfo(baseDir+"rpcVal_440.root", baseDir+"rpcVal_500pre3.root");
*/

  compareRPCInfo(baseDir+"rpcVal_53X.root", baseDir+"rpcVal_74X.root");
  //compareRPCInfo(baseDir+"rpcVal_600pre10.root", baseDir+"rpcVal_600pre8.root");
}


struct RPCRecHitInfo
{
  int region, ring, station, sector, layer, subsector, roll;
  double lx, lex;
  double gx, gy, gz;
  int clusterSize, bx;
};

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <map>
#include <set>

using namespace std;

void compareRPCInfo(TString fileName1, TString fileName2)
{
  cout << "==== Difference of " << fileName1 << " vs " << fileName2 << endl;
  
  TFile* file1 = TFile::Open(fileName1);
  TFile* file2 = TFile::Open(fileName2);

  if ( !file1 || !file1->IsOpen() ||
       !file2 || !file2->IsOpen() )
  {
    cout << "Cannot open file" << endl;
    return;
  }

  TTree* tree1 = (TTree*)file1->Get("rpcValTree/tree");
  TTree* tree2 = (TTree*)file2->Get("rpcValTree/tree");

  if ( !tree1 || !tree2 )
  {
    cout << "Cannot open tree" << endl;
    return;
  }

  // Set branches
  int run1, lumi1, event1;
  int run2, lumi2, event2;

  tree1->SetBranchAddress("runNumber"  , &run1  );
  tree1->SetBranchAddress("lumiNumber" , &lumi1 );
  tree1->SetBranchAddress("eventNumber", &event1);

  tree2->SetBranchAddress("runNumber"  , &run2  );
  tree2->SetBranchAddress("lumiNumber" , &lumi2 );
  tree2->SetBranchAddress("eventNumber", &event2);

  std::vector<RPCRecHitInfo>* recHits1 = new std::vector<RPCRecHitInfo>;
  std::vector<RPCRecHitInfo>* recHits2 = new std::vector<RPCRecHitInfo>;

  tree1->SetBranchAddress("recHits", &recHits1);
  tree2->SetBranchAddress("recHits", &recHits2);

  // Build run+event -> entry number map
  const int nEntry1 = tree1->GetEntries();
  const int nEntry2 = tree2->GetEntries();
  cout << "Getting events : " << nEntry1 << " : " << nEntry2 << endl;

  std::map<std::string, int> eventToEntry2;
  for ( int iEntry2 = 0; iEntry2 < nEntry2; ++iEntry2 )
  {
    tree2->GetEntry(iEntry2);
    eventToEntry2[Form("%d_%d", run2, event2)] = iEntry2;
  }

  cout << "Finished event to entry number mapping" << endl;

  // Start loop
  int nCommon = 0, nNoRPC = 0, nDiffSize = 0, nDiffInfo = 0;
  std::set<int> analyzedRuns;

  for ( int iEntry1 = 0; iEntry1 < nEntry1; ++iEntry1 )
  {
    const double permil = 1000.*iEntry1/nEntry1;

    tree1->GetEntry(iEntry1);

    // Find entry number with same runNumber and eventNumber
    std::map<std::string, int>::const_iterator matchedEntryItem2 = eventToEntry2.find(Form("%d_%d", run1, event1));
    if ( matchedEntryItem2 == eventToEntry2.end() ) continue;
    const int matchedEntry2 = matchedEntryItem2->second;

    cout << Form("Processed %.1f%%, % 9d/%09d, run=%07d\r", permil/10., iEntry1, nEntry1, run1);

    tree2->GetEntry(matchedEntry2);

    ++nCommon;

    if ( recHits1->empty() && recHits2->empty() )
    {
      ++nNoRPC;

      //cout << '(' << iEntry1 << ',' << matchedEntry2 << ") " << run1 << ':' << event1;
      //cout << " No RPCRecHit" << endl;
      continue;
    }
    else if ( recHits1->size() != recHits2->size() )
    {
      ++nDiffSize;

      cout << '(' << iEntry1 << ',' << matchedEntry2 << ") " << run1 << ':' << event1;
      cout << " Different size of RecHits" << endl;
      continue;
    }
    else
    {
      analyzedRuns.insert(run1);
      const int nRecHit = recHits1->size();
      for ( int iRecHit = 0; iRecHit < nRecHit; ++iRecHit )
      {
        RPCRecHitInfo& recHit1 = recHits1->at(iRecHit);
        RPCRecHitInfo& recHit2 = recHits2->at(iRecHit);

        if ( recHit1.region      != recHit2.region      ||
             recHit1.ring        != recHit2.ring        ||
             recHit1.station     != recHit2.station     ||
             recHit1.sector      != recHit2.sector      ||
             recHit1.layer       != recHit2.layer       ||
             recHit1.subsector   != recHit2.subsector   ||
             recHit1.roll        != recHit2.roll        ||
             recHit1.lx          != recHit2.lx          ||
             recHit1.lex         != recHit2.lex         ||
             recHit1.gx          != recHit2.gx          ||
             recHit1.gy          != recHit2.gy          ||
             recHit1.gz          != recHit2.gz          ||
             recHit1.clusterSize != recHit2.clusterSize ||
             recHit1.bx          != recHit2.bx          )
        {
          ++nDiffInfo;

          cout << '(' << iEntry1 << ',' << matchedEntry2 << ") " << run1 << ':' << event1;
          cout << iRecHit << "th element : " << endl;
          cout << "\tring        = " << recHit1.ring        << " " << recHit2.ring        << endl;
          cout << "\tstation     = " << recHit1.station     << " " << recHit2.station     << endl;
          cout << "\tsector      = " << recHit1.sector      << " " << recHit2.sector      << endl;
          cout << "\tlayer       = " << recHit1.layer       << " " << recHit2.layer       << endl;
          cout << "\tsubsector   = " << recHit1.subsector   << " " << recHit2.subsector   << endl;
          cout << "\troll        = " << recHit1.roll        << " " << recHit2.roll        << endl;
          cout << "\tlx          = " << recHit1.lx          << " " << recHit2.lx          << endl;
          cout << "\tlex         = " << recHit1.lex         << " " << recHit2.lex         << endl;
          cout << "\tgx          = " << recHit1.gx          << " " << recHit2.gx          << endl;
          cout << "\tgy          = " << recHit1.gy          << " " << recHit2.gy          << endl;
          cout << "\tgz          = " << recHit1.gz          << " " << recHit2.gz          << endl;
          cout << "\tclusterSize = " << recHit1.clusterSize << " " << recHit2.clusterSize << endl;
          cout << "\tbx          = " << recHit1.bx          << " " << recHit2.bx          << endl;
        }
      }
    }
  }

  cout << endl;
  cout << "======================================================\n";          
  cout << " Analyzed runs : \n";
  int i = 0;
  for ( std::set<int>::const_iterator iRun = analyzedRuns.begin();
        iRun != analyzedRuns.end(); ++iRun )
  {
    cout << *iRun << ' ';
    if ( ++i % 10 == 0 ) cout << endl;
  }
  cout << endl;
  cout << "======================================================\n";          
  cout << " Common            : " << nCommon   << endl;
  cout << " Both empty        : " << nNoRPC    << endl;
  cout << " Different number  : " << nDiffSize << endl;
  cout << " Different content : " << nDiffInfo << endl;
  cout << "======================================================\n\n";

  delete recHits1;
  delete recHits2;
}

