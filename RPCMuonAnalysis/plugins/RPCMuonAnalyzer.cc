#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

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
  ~RPCMuonOptAnalyzer() {};
  void analyze(const edm::Event& event, const edm::EventSetup& eventSetup) override;

private:
  typedef std::vector<pat::TriggerObjectStandAlone> TriggerObjects;
  typedef edm::Ref<TriggerObjects> TriggerObjectRef;
  typedef edm::Ref<pat::MuonCollection> MuonRef;

  const edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
  const edm::EDGetTokenT<edm::TriggerResults> trgToken_;
  const edm::EDGetTokenT<TriggerObjects> trgObjToken_;
  const std::string hltPathName_;
  const edm::EDGetTokenT<std::vector<pat::Muon> > muonToken_;
  const double minPt_, maxEta_;

  TTree* tree_;
  int runNumber_, lumiNumber_, eventNumber_;

  typedef std::vector<float> vfloat;
  float tag_pt_, tag_eta_, tag_phi_;
  float probe_pt_, probe_eta_, probe_phi_;
  float z_m_, z_pt_, z_eta_, z_phi_;
  int z_q_, tag_q_, probe_q_;

};

RPCMuonOptAnalyzer::RPCMuonOptAnalyzer(const edm::ParameterSet& pset):
  vertexToken_(consumes<reco::VertexCollection>(pset.getParameter<edm::InputTag>("vertex"))),
  trgToken_(consumes<edm::TriggerResults>(edm::InputTag("TriggerResults::HLT"))),
  trgObjToken_(consumes<TriggerObjects>(pset.getParameter<edm::InputTag>("triggerObject"))),
  hltPathName_(pset.getParameter<std::string>("hltPathName")),
  muonToken_(consumes<pat::MuonCollection>(pset.getParameter<edm::InputTag>("muon"))),
  minPt_(pset.getParameter<double>("minPt")),
  maxEta_(pset.getParameter<double>("maxEta"))
{
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

  tree_->Branch("probe_pt" , &probe_pt_ , "tag_pt/F" );
  tree_->Branch("probe_eta", &probe_eta_, "tag_eta/F");
  tree_->Branch("probe_phi", &probe_phi_, "tag_phi/F");
  tree_->Branch("probe_q", &probe_q_, "probe_q/I");

  tree_->Branch("z_m"  , &z_m_, "z_m/F");
  tree_->Branch("z_pt" , &z_pt_, "z_pt/F");
  tree_->Branch("z_eta", &z_eta_, "z_eta/F");
  tree_->Branch("z_phi", &z_phi_, "z_phi/F");
  tree_->Branch("z_q", &z_q_, "z_q/I");

}

void RPCMuonOptAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& eventSetup)
{
  edm::Handle<reco::VertexCollection> vertexHandle;
  event.getByToken(vertexToken_, vertexHandle);
  const reco::Vertex& vertex = vertexHandle->at(0);

  edm::Handle<edm::TriggerResults> trgHandle;
  event.getByToken(trgToken_, trgHandle);

  edm::Handle<TriggerObjects> trgObjHandle;
  event.getByToken(trgObjToken_, trgObjHandle);

  edm::Handle<pat::MuonCollection> muonHandle;
  event.getByToken(muonToken_, muonHandle);

  TriggerObjectRef trgObjRef;
  for ( int i=0, n=trgObjHandle->size(); i<n; ++i )
  {
    pat::TriggerObjectStandAlone trgObj = trgObjHandle->at(i);
    trgObj.unpackPathNames(event.triggerNames(*trgHandle));
    if ( trgObj.hasPathName(hltPathName_) )
    {
      trgObjRef = TriggerObjectRef(trgObjHandle, i);
      break;
    }
  }
  if ( trgObjRef.isNull() ) return;

  // Find tag muon
  double minDR = 1e9;
  MuonRef tagMuRef;
  for ( int i=0, n=muonHandle->size(); i<n; ++i )
  {
    MuonRef muRef(muonHandle, i);
    if ( !muRef->isTightMuon(vertex) ) continue;
    const double dR = deltaR(muRef->p4(), trgObjRef->p4());
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

  probe_pt_  = probeMuRef->pt();
  probe_eta_ = probeMuRef->eta();
  probe_phi_ = probeMuRef->phi();
  probe_q_   = probeMuRef->charge();

  z_q_ = tagMuRef->charge() + probeMuRef->charge();
  const auto zP4 = tagMuRef->p4() + probeMuRef->p4();
  z_pt_  = zP4.pt();
  z_eta_ = zP4.eta();
  z_phi_ = zP4.phi();
  z_m_   = zP4.mass();

  // Fill the ID variables
  

}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCMuonOptAnalyzer);
