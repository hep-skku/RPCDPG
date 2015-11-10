#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

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

  const edm::EDGetTokenT<edm::TriggerResults> trgToken_;
  const edm::EDGetTokenT<TriggerObjects> trgObjToken_;
  const std::string hltPathName_;
  const edm::EDGetTokenT<std::vector<pat::Muon> > muonToken_;
  const double minPt_, maxEta_;

  TTree* tree_;
  int runNumber_, lumiNumber_, eventNumber_;

  typedef std::vector<float> vfloat;
  vfloat* tag_pt_, * tag_eta_, * tag_phi_;
  vfloat* prob_pt_, * prob_eta_, * prob_phi_;

};

RPCMuonOptAnalyzer::RPCMuonOptAnalyzer(const edm::ParameterSet& pset):
  trgToken_(consumes<edm::TriggerResults>(edm::InputTag("TriggerResults::HLT"))),
  trgObjToken_(consumes<TriggerObjects>(pset.getParameter<edm::InputTag>("triggerObject"))),
  hltPathName_(pset.getParameter<std::string>("hltPathName")),
  muonToken_(consumes<pat::MuonCollection>(pset.getParameter<edm::InputTag>("muon"))),
  minPt_(pset.getParameter<double>("minPt")),
  maxEta_(pset.getParameter<double>("maxEta"))
{
  tag_pt_  = new vfloat;
  tag_eta_ = new vfloat;
  tag_phi_ = new vfloat;

  tag_pt_  = new vfloat;
  tag_eta_ = new vfloat;
  tag_phi_ = new vfloat;

  usesResource("TFileService");

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("tree", "tree");

  tree_->Branch("run", &runNumber_, "run/I");
  tree_->Branch("lumi", &lumiNumber_, "lumi/I");
  tree_->Branch("event", &eventNumber_, "event/I");

  tree_->Branch("tag_pt" , &tag_pt_ );
  tree_->Branch("tag_eta", &tag_eta_);
  tree_->Branch("tag_phi", &tag_phi_);

  tree_->Branch("prob_pt" , &prob_pt_ );
  tree_->Branch("prob_eta", &prob_eta_);
  tree_->Branch("prob_phi", &prob_phi_);

}

void RPCMuonOptAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& eventSetup)
{
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

  // Find tag muons
  double minDR = 1e9;
  MuonRef tagMuRef;
  for ( int i=0, n=muonHandle->size(); i<n; ++i )
  {
    MuonRef muRef(muonHandle, i);
    const double dR = deltaR(muRef->p4(), trgObjRef->p4());
    if ( dR < minDR )
    {
      tagMuRef = muRef;
    }
  }
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(RPCMuonOptAnalyzer);
