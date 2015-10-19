#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
//#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/TrackAssociator/interface/TrackDetectorAssociator.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "RPCDPG/RecHitAnalysis/interface/RPCRecHitInfo.h"
#include "TTree.h"

using namespace std;

class TrackToRPCNtupleMaker : public edm::one::EDAnalyzer<edm::one::SharedResources>
{
public:
  TrackToRPCNtupleMaker(const edm::ParameterSet& pset);
  ~TrackToRPCNtupleMaker();
  void analyze(const edm::Event& event, const edm::EventSetup& eventSetup);

private:
  const edm::EDGetTokenT<reco::MuonCollection> muonToken_;
  const edm::EDGetTokenT<RPCRecHitCollection> rpcRecHitToken_;
  const double minPt_, maxEta_;

  // Track Detector Associators
  TrackDetectorAssociator trackAssoc_;
  TrackAssociatorParameters trackAssocParams_;

  // Histograms and trees
  TTree* tree_;
  int runNumber_, lumiNumber_, eventNumber_;

  std::vector<RPCRefRecHitInfo>* rpcInfos_;

};

TrackToRPCNtupleMaker::TrackToRPCNtupleMaker(const edm::ParameterSet& pset):
  muonToken_(consumes<reco::MuonCollection>(edm::InputTag("muons"))),
  rpcRecHitToken_(consumes<RPCRecHitCollection>(edm::InputTag("rpcRecHits"))),
  minPt_(pset.getParameter<double>("minPt")),
  maxEta_(pset.getParameter<double>("maxEta"))
{
  auto iC = consumesCollector();
  trackAssocParams_.loadParameters(pset.getParameter<edm::ParameterSet>("TrackAssociatorParameters"), iC);

  usesResource("TFileService");

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("tree", "tree");

  tree_->Branch("run", &runNumber_, "run/I");
  tree_->Branch("lumi", &lumiNumber_, "lumi/I");
  tree_->Branch("event", &eventNumber_, "event/I");

  rpcInfos_ = new std::vector<RPCRefRecHitInfo>;
  tree_->Branch("rpcInfo", &rpcInfos_);

}

TrackToRPCNtupleMaker::~TrackToRPCNtupleMaker()
{
  if ( rpcInfos_ ) delete rpcInfos_;
}

void TrackToRPCNtupleMaker::analyze(const edm::Event& event, const edm::EventSetup& eventSetup)
{
  runNumber_ = event.id().run();
  lumiNumber_ = event.id().luminosityBlock();
  eventNumber_ = event.id().event();

  // Get the reco Muons
  edm::Handle<reco::MuonCollection> muonHandle;
  event.getByToken(muonToken_, muonHandle);
  if ( muonHandle->empty() ) return;

  // Get the RPC RecHits
  edm::Handle<RPCRecHitCollection> rpcRecHitHandle;
  event.getByToken(rpcRecHitToken_, rpcRecHitHandle);

  // Get the RPC Geometry
  edm::ESHandle<RPCGeometry> rpcGeom;
  eventSetup.get<MuonGeometryRecord>().get(rpcGeom);

  // Components to do track extrapolation
  edm::ESHandle<MagneticField> bField;
  eventSetup.get<IdealMagneticFieldRecord>().get(bField);

  edm::ESHandle<Propagator> propagator;
  eventSetup.get<TrackingComponentsRecord>().get("SteppingHelixPropagatorAny", propagator);
  trackAssoc_.setPropagator(propagator.product());

  rpcInfos_->clear();

  int nTrackerMuon = 0;
  // Extrapolate muon tracks to RPC stations
  // Make RPCDet to Muon mapping and Muon -> AssociatorInfo map
  std::map<DetId, std::vector<reco::MuonRef> > rpcDetToMuonMap;
  std::map<DetId, std::vector<TrajectoryStateOnSurface> > rpcDetToTSOSMap; // This is duplicated info, but handy
  //std::map<reco::MuonRef, std::vector<TAMuonChamberMatch> > muonToMatchMap;
  for ( size_t i=0, n=muonHandle->size(); i<n; ++i )
  {
    reco::MuonRef muRef(muonHandle, i);
    //muonToMatchMap[muRef] = std::vector<TAMuonChamberMatch>();

    // Very basic muon acceptance cut
    if ( muRef->pt() < minPt_ or std::abs(muRef->eta()) > maxEta_ ) continue;
    // And muon id cuts
    if ( !muRef->isTrackerMuon() ) continue;
    if ( !muon::isGoodMuon(*muRef, muon::TMOneStationLoose) ) continue;
    ++nTrackerMuon;

    // Get the inner track (not the global/standalone to be free from RPC info)
    const reco::TrackRef track = muRef->track();
    const auto matchInfo = trackAssoc_.associate(event, eventSetup, *track, 
                                                 trackAssocParams_, TrackDetectorAssociator::Any);
    for ( const auto& chamber : matchInfo.chambers )
    {
      if ( chamber.id.det() != DetId::Muon ) continue;
      if ( chamber.id.subdetId() != 3 ) continue;

      //muonToMatchMap[muRef].push_back(chamber);
      if ( rpcDetToMuonMap.find(chamber.id) == rpcDetToMuonMap.end() ) {
        rpcDetToMuonMap[chamber.id] = std::vector<reco::MuonRef>();
        rpcDetToTSOSMap[chamber.id] = std::vector<TrajectoryStateOnSurface>();
      }
      rpcDetToMuonMap[chamber.id].push_back(muRef);
      rpcDetToTSOSMap[chamber.id].push_back(chamber.tState);
    }
    // Sort if needed
    //std::sort(muonToMatchMap[muRef].begin(), muonToMatchMap[muRef].end(), 
    //          [](const TAMuonChamberMatch& a, const TAMuonChamberMatch& b) {
    //            return a.id.rawId() < b.id.rawId();});
  }
  if ( nTrackerMuon == 0 ) return;

  for ( auto key : rpcDetToMuonMap )
  {
    const auto& detId = key.first;
    const auto& muRefs = key.second;
    if ( muRefs.size() != 1 ) continue; // Skip if multiple muons
    auto muRef = muRefs.at(0);

    const auto& states = rpcDetToTSOSMap[detId];
    if ( states.size() != 1 ) continue; // Skip if multiple matchings

    RPCRefRecHitInfo rpcInfo;
    const RPCDetId rpcId(detId);
    rpcInfo.region    = rpcId.region()   ;
    rpcInfo.ring      = rpcId.ring()     ;
    rpcInfo.station   = rpcId.station()  ;
    rpcInfo.sector    = rpcId.sector()   ;
    rpcInfo.layer     = rpcId.layer()    ;
    rpcInfo.subsector = rpcId.subsector();
    rpcInfo.roll      = rpcId.roll()     ;

    const auto& tsos = states[0];
    const auto& refLPos = tsos.localPosition();
    const auto& refLErr = tsos.localError().positionError();
    //const auto& refLDir = tsos.localDirection();

    rpcInfo.rlx  = refLPos.x();
    rpcInfo.rly  = refLPos.y();
    rpcInfo.rlex = refLErr.xx();
    rpcInfo.mupt  = muRef->pt();
    rpcInfo.mueta = muRef->eta();
    rpcInfo.muphi = muRef->phi();

    // Find RPC RecHits in this chamber. Choose minimum |dX|
    const auto rpcHitsRange = rpcRecHitHandle->get(detId);
    double minAdx = 1e9;
    auto matchedHit = rpcHitsRange.second;
    for ( auto rpcHitItr = rpcHitsRange.first; rpcHitItr != rpcHitsRange.second; ++rpcHitItr )
    {
      // NOTE: Should we add recHit cuts?
      const double adx = std::abs(rpcHitItr->localPosition().x() - refLPos.x());
      if ( adx < minAdx )
      {
        minAdx = adx;
        matchedHit = rpcHitItr;
      }
    }

    if ( matchedHit == rpcHitsRange.second )
    {
      rpcInfo.dx = -999;
      rpcInfo.lx = rpcInfo.lex = -999;
      rpcInfo.gx = rpcInfo.gy = rpcInfo.gz = -999;
      rpcInfo.clusterSize = 0;
      rpcInfo.bx = -999;
    }
    else
    {
      // We got RPCHit here
      const LocalPoint& hitLPos = matchedHit->localPosition();
      const LocalError& hitLErr = matchedHit->localPositionError();
      const GlobalPoint& hitGPos = rpcGeom->roll(detId)->toGlobal(hitLPos);

      const double dx = hitLPos.x() - refLPos.x();
      rpcInfo.dx = dx;

      rpcInfo.lx = hitLPos.x();
      rpcInfo.gx = hitGPos.x();
      rpcInfo.gy = hitGPos.y();
      rpcInfo.gz = hitGPos.z();

      rpcInfo.lex = hitLErr.xx();
      rpcInfo.clusterSize = matchedHit->clusterSize();
      rpcInfo.bx = matchedHit->BunchX();
    }

    rpcInfos_->push_back(rpcInfo);
  }

  tree_->Fill();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(TrackToRPCNtupleMaker);

