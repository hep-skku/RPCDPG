#include "DataFormats/Common/interface/Wrapper.h"
#include "DataFormats/Common/interface/Ptr.h"

#include "RPCDPG/RecHitAnalysis/interface/RPCRecHitInfo.h"
#include <vector>

namespace {
  struct RPCDPGRecHitAnalysis_DataFormats
  {
    RPCRecHitInfo dummyRPCRecHitInfo;
    edm::Wrapper<RPCRecHitInfo> dummyRPCRecHitInfoWrapper;
    std::vector<RPCRecHitInfo> dummyRPCRecHitInfoCollection;
    edm::Wrapper<std::vector<RPCRecHitInfo> > dummyRPCRecHitInfoCollectionWrapper;
    edm::Ptr<RPCRecHitInfo> dummyRPCRecHitInfoPtr;

    RPCRefRecHitInfo dummyRPCRefRecHitInfo;
    edm::Wrapper<RPCRefRecHitInfo> dummyRPCRefRecHitInfoWrapper;
    std::vector<RPCRefRecHitInfo> dummyRPCRefRecHitInfoCollection;
    edm::Wrapper<std::vector<RPCRefRecHitInfo> > dummyRPCRefRecHitInfoCollectionWrapper;
    edm::Ptr<RPCRefRecHitInfo> dummyRPCRefRecHitInfoPtr;

  };
}
