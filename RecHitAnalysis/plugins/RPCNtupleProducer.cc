#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "RPCDPG/RecHitAnalysis/interface/RPCRecHitInfo.h"
#include "TTree.h"

#include <memory>

using namespace std;

class RPCNtupleProducer : public edm::EDAnalyzer
{
public:
  RPCNtupleProducer(const edm::ParameterSet& pset);
  ~RPCNtupleProducer();
  void analyze(const edm::Event& event, const edm::EventSetup& eventSetup);

private:
  TTree* tree_;
  int runNumber_, lumiNumber_, eventNumber_;

  std::vector<RPCRecHitInfo>* rpcRecHitInfos_;
};

RPCNtupleProducer::RPCNtupleProducer(const edm::ParameterSet& pset)
{
  rpcRecHitInfos_ = new std::vector<RPCRecHitInfo>;

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("tree", "tree");

  tree_->Branch("run", &runNumber_, "run/I");
  tree_->Branch("lumi", &lumiNumber_, "lumi/I");
  tree_->Branch("event", &eventNumber_, "event/I");
  tree_->Branch("recHits", "std::vector<RPCRecHitInfo>", rpcRecHitInfos_);
}

RPCNtupleProducer::~RPCNtupleProducer()
{
  if ( rpcRecHitInfos_ ) delete rpcRecHitInfos_;
}

void RPCNtupleProducer::analyze(const edm::Event& event, const edm::EventSetup& eventSetup)
{
  runNumber_ = event.id().run();
  lumiNumber_ = event.id().luminosityBlock();
  eventNumber_ = event.id().event();

  edm::Handle<RPCRecHitCollection> rpcRecHitHandle;
  event.getByLabel("rpcRecHits", rpcRecHitHandle);

  // Get the RPC Geometry
  edm::ESHandle<RPCGeometry> rpcGeom;
  eventSetup.get<MuonGeometryRecord>().get(rpcGeom);

  rpcRecHitInfos_->clear();

  for ( RPCRecHitCollection::const_iterator rpcRecHit = rpcRecHitHandle->begin();
        rpcRecHit != rpcRecHitHandle->end(); ++rpcRecHit )
  {
    RPCRecHitInfo rpcRecHitInfo;

    RPCDetId detId = rpcRecHit->rpcId();
    rpcRecHitInfo.region    = detId.region()   ;
    rpcRecHitInfo.ring      = detId.ring()     ;
    rpcRecHitInfo.station   = detId.station()  ;
    rpcRecHitInfo.sector    = detId.sector()   ;
    rpcRecHitInfo.layer     = detId.layer()    ;
    rpcRecHitInfo.subsector = detId.subsector();
    rpcRecHitInfo.roll      = detId.roll()     ;

    LocalPoint lp = rpcRecHit->localPosition();
    GlobalPoint gp = rpcGeom->roll(detId)->toGlobal(lp);

    rpcRecHitInfo.lx = lp.x();
    rpcRecHitInfo.gx = gp.x();
    rpcRecHitInfo.gy = gp.y();
    rpcRecHitInfo.gz = gp.z();

    rpcRecHitInfo.lex = rpcRecHit->localPositionError().xx();
    rpcRecHitInfo.clusterSize = rpcRecHit->clusterSize();
    rpcRecHitInfo.bx = rpcRecHit->BunchX();

    rpcRecHitInfos_->push_back(rpcRecHitInfo);
  }

  tree_->Fill();
}

DEFINE_FWK_MODULE(RPCNtupleProducer);

