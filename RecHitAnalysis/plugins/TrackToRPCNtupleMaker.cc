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
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
//#include "TrackingTools/TrackAssociator/interface/TrackDetectorAssociator.h"

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
  const bool doExtrapolation_;
  const edm::EDGetTokenT<reco::MuonCollection> muonToken_;
  const edm::EDGetTokenT<RPCRecHitCollection> rpcRecHitToken_;
  const double minPt_, maxEta_;

  // Histograms and trees
  TTree* tree_;
  int runNumber_, lumiNumber_, eventNumber_;

  std::vector<RPCRefRecHitInfo>* rpcInfos_;

  struct ExtPoint
  {
    float lx, ly;
    float lxErr;
    float gx, gy, gz;
  };

};

TrackToRPCNtupleMaker::TrackToRPCNtupleMaker(const edm::ParameterSet& pset):
  doExtrapolation_(pset.getParameter<bool>("doExtrapolation")),
  muonToken_(consumes<reco::MuonCollection>(edm::InputTag("muons"))),
  rpcRecHitToken_(consumes<RPCRecHitCollection>(edm::InputTag("rpcRecHits"))),
  minPt_(pset.getParameter<double>("minPt")),
  maxEta_(pset.getParameter<double>("maxEta"))
{
  usesResource("TFileService");

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("tree", "tree");

  tree_->Branch("run", &runNumber_, "run/I");
  tree_->Branch("lumi", &lumiNumber_, "lumi/I");
  tree_->Branch("event", &eventNumber_, "event/I");

  rpcInfos_ = new std::vector<RPCRefRecHitInfo>;
  tree_->Branch("rpcInfo", "std::vector<RPCRefRecHitInfo>", &rpcInfos_);

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

  // Get the RPC RecHits
  edm::Handle<RPCRecHitCollection> rpcRecHitHandle;
  event.getByToken(rpcRecHitToken_, rpcRecHitHandle);

  // Get the reco Muons
  edm::Handle<reco::MuonCollection> muonHandle;
  event.getByToken(muonToken_, muonHandle);
  if ( muonHandle->empty() ) return;

  // Get the RPC Geometry
  edm::ESHandle<DTGeometry> dtGeom;
  edm::ESHandle<CSCGeometry> cscGeom;
  edm::ESHandle<RPCGeometry> rpcGeom;
  eventSetup.get<MuonGeometryRecord>().get(dtGeom);
  eventSetup.get<MuonGeometryRecord>().get(cscGeom);
  eventSetup.get<MuonGeometryRecord>().get(rpcGeom);

  // Components to do track extrapolation
  edm::ESHandle<MagneticField> bField;
  edm::ESHandle<TransientTrackBuilder> transTrackBuilder;
  edm::ESHandle<Propagator> propagator;
  if ( doExtrapolation_ )
  {
    eventSetup.get<IdealMagneticFieldRecord>().get(bField);
    eventSetup.get<TransientTrackRecord>().get("TransientTrackBuilder", transTrackBuilder);
    eventSetup.get<TrackingComponentsRecord>().get("SteppingHelixPropagatorAny", propagator);
  }

  rpcInfos_->clear();

  int nTrackerMuon = 0;
  // Extrapolate muon tracks to RPC stations
  // Make RPCDet to Muon mapping and Muon -> AssociatorInfo map
  std::map<DetId, std::vector<reco::MuonRef> > rpcDetToMuonMap;
  std::map<DetId, std::vector<ExtPoint> > rpcDetToPointMap; // This is duplicated info, but handy
  for ( size_t i=0, n=muonHandle->size(); i<n; ++i )
  {
    reco::MuonRef muRef(muonHandle, i);

    // Very basic muon acceptance cut
    if ( muRef->pt() < minPt_ or std::abs(muRef->eta()) > maxEta_ ) continue;
    // And muon id cuts
    if ( !muRef->isTrackerMuon() ) continue;
    if ( !muon::isGoodMuon(*muRef, muon::TMOneStationLoose) ) continue;
    ++nTrackerMuon;

    // Get the inner track (not the global/standalone to be free from RPC info)
    const reco::TrackRef track = muRef->track();
    if ( track.isNull() ) continue;

    // Define eta phi range from muon match information, to narrow down interested RPC rolls
    std::vector<std::tuple<GlobalPoint, double, double> > matchGPs;
    for( auto& match : muRef->matches() )
    {
      if ( match.detector() == MuonSubdetId::RPC ) continue;

      const LocalPoint localPos(match.x, match.y);
      if ( match.detector() == MuonSubdetId::DT )
      {
        const auto gPos = dtGeom->chamber(match.id)->toGlobal(localPos);
        matchGPs.push_back(std::make_tuple(gPos, match.xErr, match.yErr));
      }
      else if ( match.detector() == MuonSubdetId::CSC )
      {
        const auto gPos = cscGeom->chamber(match.id)->toGlobal(localPos);
        matchGPs.push_back(std::make_tuple(gPos, match.xErr, match.yErr));
      }
    }

    // Find nearby rolls
    std::vector<const RPCRoll*> matchedRolls;
    for ( const RPCRoll* roll : rpcGeom->rolls() )
    {
      if ( !roll ) continue;
      const auto& surface = roll->surface();

      bool hasNearbyMatch = false;
      for ( const auto& x : matchGPs )
      {
        const auto matchLPos = roll->toLocal(std::get<0>(x));
        if ( surface.bounds().inside(Local2DPoint(matchLPos.x(), matchLPos.y())) )
        {
          hasNearbyMatch = true;
          break;
        }
      }
      if ( !hasNearbyMatch ) continue;

      matchedRolls.push_back(roll);
    }
    // Sort rolls by distance from the centre
    std::sort(matchedRolls.begin(), matchedRolls.end(),
              [](const RPCRoll* a, const RPCRoll* b) { return a->position().mag2() < a->position().mag2(); });

    if ( doExtrapolation_ )
    {
      // Get the inner track (not the global/standalone to be free from RPC info)
      const reco::TrackRef track = muRef->track();
      if ( track.isNull() ) continue;
      // Prepare extrapolation onto RPC rolls. Currently we are extrapolating onto all RPC rolls, but this can be improved
      const reco::TransientTrack transTrack = transTrackBuilder->build(track);
      if ( !transTrack.isValid() ) continue;
      TrajectoryStateOnSurface prevState = transTrack.outermostMeasurementState();
      if ( !prevState.isValid() ) continue;
      for ( const RPCRoll* roll : matchedRolls )
      {
        const auto surface = roll->surface();
        const auto tState = propagator->propagate(prevState, surface);
        if ( !tState.isValid() ) continue;

        const auto lPos = tState.localPosition();
        if ( !surface.bounds().inside(lPos) ) continue;

        const auto id = roll->id();
        //cout << stateOnRPC.localPosition().x() << endl;
        if ( rpcDetToMuonMap.find(id) == rpcDetToMuonMap.end() )
        {
          rpcDetToMuonMap[id] = std::vector<reco::MuonRef>();
          rpcDetToPointMap[id] = std::vector<ExtPoint>();
        }

        const auto gPos = tState.globalPosition();
        const ExtPoint point = {lPos.x(), lPos.y(),
                                std::sqrt(tState.localError().positionError().xx()),
                                gPos.x(), gPos.y(), gPos.z()};
        rpcDetToMuonMap[id].push_back(muRef);
        rpcDetToPointMap[id].push_back(point);

        prevState = tState;
      }
    }
    else
    {
      // Get the pre-calculated extrapolated points from RPCMuon reconstruction
      for ( const auto& chMatch : muRef->matches() )
      {
        if ( chMatch.detector() != MuonSubdetId::RPC ) continue;
        const auto& id = chMatch.id;
        if ( !rpcGeom->roll(id) ) continue;

        if ( rpcDetToMuonMap.find(id) == rpcDetToMuonMap.end() )
        {
          rpcDetToMuonMap[id] = std::vector<reco::MuonRef>();
          rpcDetToPointMap[id] = std::vector<ExtPoint>();
        }
        const LocalPoint refLPos(chMatch.x, chMatch.y);
        const GlobalPoint refGPos = rpcGeom->roll(id)->toGlobal(refLPos);
        const ExtPoint point = {chMatch.x, chMatch.y, chMatch.xErr,
                                refGPos.x(), refGPos.y(), refGPos.z()};
        rpcDetToMuonMap[id].push_back(muRef);
        rpcDetToPointMap[id].push_back(point);
      }
    }
  }
  if ( nTrackerMuon == 0 ) return;

  for ( auto key : rpcDetToMuonMap )
  {
    const auto& detId = key.first;
    const auto& muRefs = key.second;
    if ( muRefs.size() != 1 ) continue; // Skip if multiple muons
    auto muRef = muRefs.at(0);

    RPCRefRecHitInfo rpcInfo;
    const RPCDetId rpcId(detId);
    rpcInfo.region    = rpcId.region()   ;
    rpcInfo.ring      = rpcId.ring()     ;
    rpcInfo.station   = rpcId.station()  ;
    rpcInfo.sector    = rpcId.sector()   ;
    rpcInfo.layer     = rpcId.layer()    ;
    rpcInfo.subsector = rpcId.subsector();
    rpcInfo.roll      = rpcId.roll()     ;

    rpcInfo.mupt  = muRef->pt();
    rpcInfo.mueta = muRef->eta();
    rpcInfo.muphi = muRef->phi();

    rpcInfo.dx = -999;
    rpcInfo.lx = rpcInfo.lex = -999;
    rpcInfo.gx = rpcInfo.gy = rpcInfo.gz = -999;
    rpcInfo.clusterSize = 0;
    rpcInfo.bx = -999;
    rpcInfo.lex = -999;

    const auto& points = rpcDetToPointMap[detId];
    if ( points.size() != 1 ) continue; // Skip if multiple matchings
    const auto& point = points[0];

    rpcInfo.rlx  = point.lx;
    rpcInfo.rly  = point.ly;
    rpcInfo.rlex = point.lxErr;
    rpcInfo.rgx  = point.gx;
    rpcInfo.rgy  = point.gy;
    rpcInfo.rgz  = point.gz;

    // Find RPC RecHits in this chamber. Choose minimum |dX|
    const auto rpcHitsRange = rpcRecHitHandle->get(detId);
    double minAdx = 1e9;
    auto matchedHit = rpcHitsRange.second;
    for ( auto rpcHitItr = rpcHitsRange.first;
          rpcHitItr != rpcHitsRange.second; ++rpcHitItr )
    {
      // NOTE: Should we add recHit cuts?
      const double adx = std::abs(rpcHitItr->localPosition().x() - point.lx);
      if ( adx < minAdx )
      {
        minAdx = adx;
        matchedHit = rpcHitItr;
      }
    }

    if ( matchedHit != rpcHitsRange.second )
    {
      // We got RPCHit here
      const LocalPoint& hitLPos = matchedHit->localPosition();
      const LocalError& hitLErr = matchedHit->localPositionError();
      const GlobalPoint hitGPos = rpcGeom->roll(detId)->toGlobal(hitLPos);

      rpcInfo.dx = hitLPos.x() - point.lx;

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

