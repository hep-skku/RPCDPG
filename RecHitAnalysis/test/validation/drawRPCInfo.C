#include "TString.h"
#include "TH1F.h"
#include "TPaveStats.h"

void plot2(TH1F* h1, TH1F* h2);
void drawRPCInfo(TString fileName1, TString fileName2);

void drawRPCInfo()
{
  TString baseDir = "/store/data/jhgoh/CMS/ntuple/RPC/DataVsDataValidation/";

  drawRPCInfo(baseDir+"rpcVal_53X.root", baseDir+"rpcVal_74X.root");
}

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"

#include <iostream>
#include <map>
#include <set>

using namespace std;

void drawRPCInfo(TString fileName1, TString fileName2)
{
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

  TH1F* hregion1    = new TH1F("hregion1"   , "region"        ,   3,  -1.5,  1.5);
  TH1F* hring1      = new TH1F("hring1"     , "ring"          ,   5,  -2.5,  2.5);
  TH1F* hstation1   = new TH1F("hstation1"  , "station"       ,   5,  -0.5,  4.5);
  TH1F* hsector1    = new TH1F("hsector1"   , "sector"        ,  13,  -0.5, 12.5);
  TH1F* hlayer1     = new TH1F("hlayer1"    , "layer"         ,   2,   0.5,  2.5);
  TH1F* hsubsector1 = new TH1F("hsubsector1", "subsector"     ,   6,   0.5,  6.5);
  TH1F* hroll1      = new TH1F("hroll1"     , "roll"          ,   3,   0.5,  3.5);
  TH1F* hlx1        = new TH1F("hlx1"       , "localX"        , 100,  -150,  150);
  TH1F* hlex1       = new TH1F("hlex1"      , "localErrorX"   , 100,     0,   10);
  TH1F* hgx1        = new TH1F("hgx1"       , "global X"      , 100,  -800,  800);
  TH1F* hgy1        = new TH1F("hgy1"       , "global Y"      , 100,  -800,  800);
  TH1F* hgz1        = new TH1F("hgz1"       , "global Z"      , 100, -1000, 1000);
  TH1F* hcls1       = new TH1F("hcls1"      , "Cluster size"  ,  50,     0,   50);
  TH1F* hbx1        = new TH1F("hbx1"       , "bunch crossing",   7,  -3.5,  3.5);

  TH1F* hregion2    = new TH1F("hregion2"   , "region"        ,   3,  -1.5,  1.5);
  TH1F* hring2      = new TH1F("hring2"     , "ring"          ,   5,  -2.5,  2.5);
  TH1F* hstation2   = new TH1F("hstation2"  , "station"       ,   5,  -0.5,  4.5);
  TH1F* hsector2    = new TH1F("hsector2"   , "sector"        ,  13,  -0.5, 12.5);
  TH1F* hlayer2     = new TH1F("hlayer2"    , "layer"         ,   2,   0.5,  2.5);
  TH1F* hsubsector2 = new TH1F("hsubsector2", "subsector"     ,   6,   0.5,  6.5);
  TH1F* hroll2      = new TH1F("hroll2"     , "roll"          ,   3,   0.5,  3.5);
  TH1F* hlx2        = new TH1F("hlx2"       , "localX"        , 100,  -150,  150);
  TH1F* hlex2       = new TH1F("hlex2"      , "localErrorX"   , 100,     0,   10);
  TH1F* hgx2        = new TH1F("hgx2"       , "global X"      , 100,  -800,  800);
  TH1F* hgy2        = new TH1F("hgy2"       , "global Y"      , 100,  -800,  800);
  TH1F* hgz2        = new TH1F("hgz2"       , "global Z"      , 100, -1000, 1000);
  TH1F* hcls2       = new TH1F("hcls2"      , "Cluster size"  ,  50,     0,   50);
  TH1F* hbx2        = new TH1F("hbx2"       , "bunch crossing",   7,  -3.5,  3.5);

  tree1->Draw("recHits.region>>hregion1"      , "", "goff");
  tree1->Draw("recHits.ring>>hring1"          , "", "goff");
  tree1->Draw("recHits.station>>hstation1"    , "", "goff");
  tree1->Draw("recHits.sector>>hsector1"      , "", "goff");
  tree1->Draw("recHits.layer>>hlayer1"        , "", "goff");
  tree1->Draw("recHits.subsector>>hsubsector1", "", "goff");
  tree1->Draw("recHits.roll>>hroll1"          , "", "goff");
  tree1->Draw("recHits.lx>>hlx1"              , "", "goff");
  tree1->Draw("recHits.lex>>hlex1"            , "", "goff");
  tree1->Draw("recHits.gx>>hgx1"              , "", "goff");
  tree1->Draw("recHits.gy>>hgy1"              , "", "goff");
  tree1->Draw("recHits.gz>>hgz1"              , "", "goff");
  tree1->Draw("recHits.clusterSize>>hcls1"    , "", "goff");
  tree1->Draw("recHits.bx>>hbx1"              , "", "goff");

  tree2->Draw("recHits.region>>hregion2"      , "", "goff");
  tree2->Draw("recHits.ring>>hring2"          , "", "goff");
  tree2->Draw("recHits.station>>hstation2"    , "", "goff");
  tree2->Draw("recHits.sector>>hsector2"      , "", "goff");
  tree2->Draw("recHits.layer>>hlayer2"        , "", "goff");
  tree2->Draw("recHits.subsector>>hsubsector2", "", "goff");
  tree2->Draw("recHits.roll>>hroll2"          , "", "goff");
  tree2->Draw("recHits.lx>>hlx2"              , "", "goff");
  tree2->Draw("recHits.lex>>hlex2"            , "", "goff");
  tree2->Draw("recHits.gx>>hgx2"              , "", "goff");
  tree2->Draw("recHits.gy>>hgy2"              , "", "goff");
  tree2->Draw("recHits.gz>>hgz2"              , "", "goff");
  tree2->Draw("recHits.clusterSize>>hcls2"    , "", "goff");
  tree2->Draw("recHits.bx>>hbx2"              , "", "goff");

  plot2(hregion1   , hregion2   );
  plot2(hring1     , hring2     );
  plot2(hstation1  , hstation2  );
  plot2(hsector1   , hsector2   );
  plot2(hlayer1    , hlayer2    );
  plot2(hsubsector1, hsubsector2);
  plot2(hroll1     , hroll2     );
  plot2(hlx1       , hlx2       );
  plot2(hlex1      , hlex2      );
  plot2(hgx1       , hgx2       );
  plot2(hgy1       , hgy2       );
  plot2(hgz1       , hgz2       );
  plot2(hcls1      , hcls2      );
  plot2(hbx1       , hbx2       );
}

void plot2(TH1F* h1, TH1F* h2)
{
  TCanvas* c = new TCanvas(TString("c")+h1->GetName(), h1->GetName(), 500, 500);
  h1->SetLineColor(kBlue);
  h2->SetLineColor(kRed);

  h1->Draw();
  h2->Draw("sames");

  TPaveStats* ps1 = (TPaveStats *)h1->GetListOfFunctions()->FindObject("stats");
  TPaveStats* ps2 = (TPaveStats *)h2->GetListOfFunctions()->FindObject("stats");

  if ( ps1 )
  {
    ps1->SetLineColor(kBlue);
    ps1->SetX1NDC(0.6);
    ps1->SetX2NDC(0.9);
    ps1->SetY1NDC(0.7);
    ps1->SetY2NDC(0.9);
  }
  if ( ps2 )
  {
    ps2->SetLineColor(kRed);
    ps2->SetX1NDC(0.6);
    ps2->SetX2NDC(0.9);
    ps2->SetY1NDC(0.5);
    ps2->SetY2NDC(0.7);
  }
}

