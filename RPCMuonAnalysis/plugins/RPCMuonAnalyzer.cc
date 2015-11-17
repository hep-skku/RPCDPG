#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "TTree.h"

using namespace std;

class RPCMuonOptAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>
{
public:
  RPCMuonOptAnalyzer(const edm::ParameterSet& pset);
  ~RPCMuonOptAnalyzer();
  void analyze(const edm::Event& event, const edm::EventSetup& eventSetup) override;

private:
  bool printHLTNames_;

  typedef std::vector<pat::TriggerObjectStandAlone> TriggerObjects;
  typedef edm::Ref<TriggerObjects> TriggerObjectRef;
  typedef edm::Ref<pat::MuonCollection> MuonRef;
  typedef std::vector<std::string> vstring;
  typedef std::vector<float> vfloat;

  const edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
  const edm::EDGetTokenT<edm::TriggerResults> trigToken_;
  const edm::EDGetTokenT<TriggerObjects> trigObjToken_;
  const vstring hltPrefixes_;
  const edm::EDGetTokenT<std::vector<pat::Muon> > muonToken_;
  const double minPt_, maxEta_;

  TTree* tree_;
  int runNumber_, lumiNumber_, eventNumber_;

  float tag_pt_, tag_eta_, tag_phi_;
  float pt_, eta_, phi_;
  float z_m_, z_pt_, z_eta_, z_phi_;
  int z_q_, tag_q_, q_;

  bool isGLB_, isTRK_, isSTA_, isRPC_, isPF_;
  bool isTight_, isMedium_, isLoose_, isRPCLoose_;
  bool isTMOneLoose_, isTMArb_;
  int nMatch_, nStation_, nRPCLayer_;

  vfloat* dxs_, * pulls_;
};

RPCMuonOptAnalyzer::RPCMuonOptAnalyzer(const edm::ParameterSet& pset):
  printHLTNames_(true),
  vertexToken_(consumes<reco::VertexCollection>(pset.getParameter<edm::InputTag>("vertex"))),
  trigToken_(consumes<edm::TriggerResults>(edm::InputTag("TriggerResults::HLT"))),
  trigObjToken_(consumes<TriggerObjects>(pset.getParameter<edm::InputTag>("triggerObject"))),
  hltPrefixes_(pset.getParameter<vstring>("hltPrefixes")),
  muonToken_(consumes<pat::MuonCollection>(pset.getParameter<edm::InputTag>("muon"))),
  minPt_(pset.getParameter<double>("minPt")),
  maxEta_(pset.getParameter<double>("maxEta"))
{
  dxs_ = new vfloat;
  pulls_ = new vfloat;

  usesResource("TFileService");

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("tree", "tree");

  tree_->Branch("run", &runNumber_, "run/I");
  tree_->Branch("lumi", &lumiNumber_, "lumi/I");
  tree_->Branch("event", &eventNumber_, "event/I");

  tree_->Branch("tag_pt" , &tag_pt_ , "tag_pt/F" );
  tree_->Branch("tag_eta", &tag_eta_, "tag_eta/F");
  tree_->Branch("tag_phi", &tag_phi_, "tag_phi/F");
  tree_->Branch("tag_q", &tag_q_, "tag_q/I");

  tree_->Branch("z_m"  , &z_m_, "z_m/F");
  tree_->Branch("z_pt" , &z_pt_, "z_pt/F");
  tree_->Branch("z_eta", &z_eta_, "z_eta/F");
  tree_->Branch("z_phi", &z_phi_, "z_phi/F");
  tree_->Branch("z_q", &z_q_, "z_q/I");

  tree_->Branch("pt" , &pt_ , "pt/F" );
  tree_->Branch("eta", &eta_, "eta/F");
  tree_->Branch("phi", &phi_, "phi/F");
  tree_->Branch("q", &q_, "q/I");

  tree_->Branch("isGLB", &isGLB_, "isGLB/O");
  tree_->Branch("isTRK", &isTRK_, "isTRK/O");
  tree_->Branch("isSTA", &isSTA_, "isSTA/O");
  tree_->Branch("isPF" , &isPF_ , "isPF/O ");
  tree_->Branch("isRPC", &isRPC_, "isRPC/O");

  tree_->Branch("isTight"   , &isTight_   , "isTight/O"   );
  tree_->Branch("isMedium"  , &isMedium_  , "isMedium/O"  );
  tree_->Branch("isLoose"   , &isLoose_   , "isLoose/O"   );
  tree_->Branch("isRPCLoose", &isRPCLoose_, "isRPCLoose/O");
  tree_->Branch("isTMArb", &isTMArb_, "isTMArb/O");
  tree_->Branch("isTMOneLoose", &isTMOneLoose_, "isTMOneLoose/O");

  tree_->Branch("nMatch"   , &nMatch_   , "nMatch/I"   );
  tree_->Branch("nStation" , &nStation_ , "nStation/I" );
  tree_->Branch("nRPCLayer", &nRPCLayer_, "nRPCLayer/I");

  tree_->Branch("dxs", "std::vector<float>", &dxs_);
  tree_->Branch("pulls", "std::vector<float>", &pulls_);

}

RPCMuonOptAnalyzer::~RPCMuonOptAnalyzer()
{
  delete dxs_;
  delete pulls_;
}

void RPCMuonOptAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& eventSetup)
{
  edm::Handle<reco::VertexCollection> vertexHandle;
  event.getByToken(vertexToken_, vertexHandle);
  const reco::Vertex& vertex = vertexHandle->at(0);

  edm::Handle<edm::TriggerResults> trigHandle;
  event.getByToken(trigToken_, trigHandle);

  edm::Handle<TriggerObjects> trigObjHandle;
  event.getByToken(trigObjToken_, trigObjHandle);

  edm::Handle<pat::MuonCollection> muonHandle;
  event.getByToken(muonToken_, muonHandle);

  // Print out trigger names for an information
  const auto triggerNames = event.triggerNames(*trigHandle);
  if ( printHLTNames_ )
  {
    for ( auto& x : triggerNames.triggerNames() ) cout << x << endl;
    printHLTNames_ = false;
  }

  // Pick up a trigger object to be used tag muon selection
  TriggerObjectRef trigObjRef;
  for ( int i=0, n=trigObjHandle->size(); i<n; ++i )
  {
    pat::TriggerObjectStandAlone trigObj = trigObjHandle->at(i);
    trigObj.unpackPathNames(triggerNames);
    for ( const auto& path : trigObj.pathNames() )
    {
      for ( const auto& hltPrefix : hltPrefixes_ )
      {
        const auto matching = std::mismatch(hltPrefix.begin(), hltPrefix.end(), path.begin());
        if ( matching.first == hltPrefix.end() )
        {
          trigObjRef = TriggerObjectRef(trigObjHandle, i);
          break;
        }
      }
      if ( trigObjRef.isNonnull() ) break;
    }
  }
  if ( trigObjRef.isNull() ) return;

  // Find tag muon
  double minDR = 1e9;
  MuonRef tagMuRef;
  for ( int i=0, n=muonHandle->size(); i<n; ++i )
  {
    MuonRef muRef(muonHandle, i);
    if ( !muRef->isTightMuon(vertex) ) continue;
    const double dR = deltaR(muRef->p4(), trigObjRef->p4());
    if ( dR < minDR )
    {
      tagMuRef = muRef;
    }
  }
  if ( tagMuRef.isNull() ) return;

  // Find probe muon
  MuonRef probeMuRef;
  for ( int i=0, n=muonHandle->size(); i<n; ++i )
  {
    MuonRef muRef(muonHandle, i);
    if ( muRef == tagMuRef ) continue;

    probeMuRef = muRef;
    break;
  }
  if ( probeMuRef.isNull() ) return;

  // Fill the tree
  tag_pt_  = tagMuRef->pt();
  tag_eta_ = tagMuRef->eta();
  tag_phi_ = tagMuRef->phi();
  tag_q_   = tagMuRef->charge();

  pt_  = probeMuRef->pt();
  eta_ = probeMuRef->eta();
  phi_ = probeMuRef->phi();
  q_   = probeMuRef->charge();

  z_q_ = tagMuRef->charge() + probeMuRef->charge();
  const auto zP4 = tagMuRef->p4() + probeMuRef->p4();
  z_pt_  = zP4.pt();
  z_eta_ = zP4.eta();
  z_phi_ = zP4.phi();
  z_m_   = zP4.mass();

  // Fill the ID variables
  isGLB_ = probeMuRef->isGlobalMuon();
  isTRK_ = probeMuRef->isTrackerMuon();
  isSTA_ = probeMuRef->isStandAloneMuon();
  isPF_  = probeMuRef->isPFMuon();
  isRPC_ = probeMuRef->isRPCMuon();

  isTight_ = probeMuRef->isTightMuon(vertex);
  isMedium_ = probeMuRef->isMediumMuon();
  isLoose_ = probeMuRef->isLooseMuon();

  isRPCLoose_ = muon::isGoodMuon(*probeMuRef, muon::RPCMuLoose, reco::Muon::RPCHitAndTrackArbitration);
  isTMArb_ = muon::isGoodMuon(*probeMuRef, muon::TrackerMuonArbitrated, reco::Muon::SegmentAndTrackArbitration);
  isTMOneLoose_ = muon::isGoodMuon(*probeMuRef, muon::TMOneStationLoose, reco::Muon::SegmentAndTrackArbitration);

  nMatch_ = probeMuRef->numberOfMatches();
  nStation_ = probeMuRef->numberOfMatchedStations();
  nRPCLayer_ = probeMuRef->numberOfMatchedRPCLayers();

  dxs_->clear();
  pulls_->clear();
  for ( const auto& chMatch : probeMuRef->matches() )
  {
    if ( chMatch.detector() != MuonSubdetId::RPC ) continue;

    const double refX = chMatch.x;
    const double refXErr = chMatch.xErr;
    double minDX = 1e9;
    for ( const auto& rpcMatch : chMatch.rpcMatches )
    {
      const double x = rpcMatch.x;
      const double dx = x-refX;
      if ( std::abs(dx) < std::abs(minDX) ) minDX = dx;
    }

    if ( minDX < 1e9 )
    {
      dxs_->push_back(minDX);
      pulls_->push_back(refXErr != 0 ? minDX/refXErr : 1e9);
    }
  }
  
  // Finalize tree
  tree_->Fill();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCMuonOptAnalyzer);
