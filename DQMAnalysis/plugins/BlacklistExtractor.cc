#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

#include <string>
#include <fstream>
using namespace std;

class BlacklistExtractor : public DQMEDHarvester
{
public:
  BlacklistExtractor(const edm::ParameterSet& ps);
  ~BlacklistExtractor() {};

  void dqmEndLuminosityBlock(DQMStore::IBooker&, DQMStore::IGetter&, const edm::LuminosityBlock&, const edm::EventSetup&) override;
  void dqmEndJob(DQMStore::IBooker &, DQMStore::IGetter &) override;

private:
  string folderName_;
  string outFileName_;
  unsigned int minEvents_;
  int run_;
  double minDeadFrac_;
};

BlacklistExtractor::BlacklistExtractor(const edm::ParameterSet& ps)
{
  folderName_ = ps.getUntrackedParameter<string>("folder", "RPC/AllHits");
  outFileName_ = ps.getUntrackedParameter<string>("outFile", "Blacklist.txt");
  minEvents_ = ps.getUntrackedParameter<unsigned int>("minEvents", 10000);
  minDeadFrac_ = ps.getUntrackedParameter<double>("minDeadFrac", 0.8); // Minimum dead fraction to put blacklist

  run_ = -1;
}

void BlacklistExtractor::dqmEndLuminosityBlock(DQMStore::IBooker&, DQMStore::IGetter&,
                                               const edm::LuminosityBlock& lumi, const edm::EventSetup&)
{
  const int run = lumi.id().run();
  if ( run_ == -1 ) run_ = run;
  else if ( run_ != run )
  {
    cout << "Update run number " << run_ << "->" << run << endl;
    run_ = run;
  }
}

void BlacklistExtractor::dqmEndJob(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter)
{
  const char* fstr = folderName_.c_str();
  char buffer[101];

  //snprintf(buffer, 100, "blacklist_%d.txt", run_);
  ofstream fout(outFileName_);
  fout << "# Run = " << run_ << endl;

  snprintf(buffer, 100, "%s/RPCEvents", folderName_.c_str());
  const auto meRPCEvents = igetter.get(buffer);
  if ( !meRPCEvents or meRPCEvents->getBinContent(1) < minEvents_ ) return;

  static const char* chStrsBarrel[] = {
    "W%+d_RB1in_S%02d", // B, F
    "W%+d_RB1out_S%02d", // B, F
    "W%+d_RB2in_S%02d", // B, M, F
    "W%+d_RB2out_S%02d", // B, F
    "W%+d_RB3-_S%02d", // B, F
    "W%+d_RB3+_S%02d", // B, F
    "W%+d_RB4-_S%02d", // B, F
    "W%+d_RB4+_S%02d", // B, F
    "W%+d_RB4--_S%02d", // B, F
    "W%+d_RB4++_S%02d" // B, F
  };
  static const char* chSuffixBarrel[] = {"B", "F", "M"};
  static const char* chSuffixEndcap[] = {"A", "B", "C"};

  // Load wheels
  for ( int wheel = -2; wheel <= 2; ++wheel )
  {
    snprintf(buffer, 100, "%s/Barrel/Wheel_%d", fstr, wheel);
    const string dirWheel(buffer);

    // Get the strip occupancy plots
    for ( int sector = 1; sector <= 12; ++sector )
    {
      const int nRollsTotal = (sector == 4) ? 21 : (sector == 9 or sector == 11 ) ? 15 : 17;
      // This for loop condition is quite unusually complicated,
      // termination criteria works with combining inner-istrip loop
      for ( int iroll = 1, ichStr = 0, station = 0; iroll <= nRollsTotal; ++ichStr )
      {
        if ( ichStr < 8 and ichStr % 2 == 0 ) ++station;

        snprintf(buffer, 100, chStrsBarrel[ichStr], wheel, sector);
        const string chName(buffer);
        
        snprintf(buffer, 100, "%s/sector_%d/station_%d/Occupancy_%s", dirWheel.c_str(), sector, station, chName.c_str());
        MonitorElement* srcME = igetter.get(buffer);
        if ( !srcME ) break;
        const auto hSrc = srcME->getTH1();
        
        const int nstrip = hSrc->GetNbinsX();

        const int nroll = ichStr == 2 ? 3 : 2; // RB2in is divide by 3 rolls
        const int nstripPerRoll = nstrip/nroll;
        for ( int iiroll = 0; iiroll < nroll; ++iiroll )
        {
          double ndead = 0;
          for ( int istrip = 0; istrip < nstripPerRoll; ++istrip )
          {
            const int bin = 1+istrip+iiroll*nstripPerRoll;
            if ( hSrc->GetBinContent(bin) == 0 ) ndead += 1;
          }

          const double fdead = ndead/nstripPerRoll;
          if ( fdead > minDeadFrac_ )
          {
            fout << chName << "_";
            if ( ichStr != 2 ) fout << chSuffixBarrel[iiroll]; // 1,2->0,1->"B","F"
            else fout << chSuffixBarrel[(iiroll*2+3)%3]; // 0,1,2->0,2,4->3,5,7->0,2,1->"B","M","F"
            fout << " " << fdead << endl;
          }
        } // roll in a chamber
      } // roll (and chamber)
    } // sector
  } // wheel

  // Load disks
  for ( int disk = -4; disk <= 4; ++disk )
  {
    if ( disk == 0 ) continue;

    if ( disk > 0 ) snprintf(buffer, 100, "%s/Endcap+/Disk_%d", fstr, disk);
    else snprintf(buffer, 100, "%s/Endcap-/Disk_%d", fstr, disk);
    const string dirDisk(buffer);

    // Get the strip occupancy plots
    for ( int ring = 2; ring <= 3; ++ring )
    {
      for ( int sector = 1; sector <= 6; ++sector )
      {
        for ( int segment = 6*(sector-1)+1, segmentMax = 6*sector; segment <= segmentMax; ++segment )
        {
          snprintf(buffer, 100, "RE%+d_R%d_CH%02d", disk, ring, segment);
          const string chName(buffer);
          snprintf(buffer, 100, "%s/ring_%d/sector_%d/Occupancy_%s", dirDisk.c_str(), ring, sector, chName.c_str());
          MonitorElement* srcME = igetter.get(buffer);
          if ( !srcME ) break;
          const auto hSrc = srcME->getTH1();

          const int nstrip = hSrc->GetNbinsX();
          const int nstripPerRoll = nstrip/3; // Endcap chambers are consist of 3 rolls
          for ( int iiroll = 0; iiroll < 3; ++iiroll )
          {
            double ndead = 0;
            for ( int istrip = 0; istrip < nstripPerRoll; ++istrip )
            {
              const int bin = 1+istrip+iiroll*nstripPerRoll;
              if ( hSrc->GetBinContent(bin) == 0 ) ndead += 1;
            }

            const double fdead = ndead/nstripPerRoll;
            if ( fdead > minDeadFrac_ )
            {
              fout << chName << "_" << chSuffixEndcap[iiroll] << " " << fdead << endl;
            }
          } // strip
        } // chamber (or segment)
      } // sector
    } // ring
  } // disk
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(BlacklistExtractor);
