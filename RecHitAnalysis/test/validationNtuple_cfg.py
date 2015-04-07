import FWCore.ParameterSet.Config as cms

process = cms.Process("RPCRecHitValidation")

### standard includes
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("Configuration.StandardSequences.Services_cff")
process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("Configuration.StandardSequences.RawToDigi_cff")

process.MessageLogger.cerr.FwkReport.reportEvery = 1000

### conditions
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.PyReleaseValidation.autoCond import autoCond
process.GlobalTag.globaltag = autoCond['startup']

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(),
    secondaryFileNames = cms.untracked.vstring()
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("rpcVal.root")
)

process.source.fileNames.extend([
  'rfio:/castor/cern.ch/cms/store/relval/CMSSW_4_4_0_pre7/Mu/RECO/GR_R_44_V1_mu2010B-v1/0144/24A3B9F4-F8BD-E011-BDE0-0026189438D5.root',
])

process.rpcValTree = cms.EDAnalyzer("RPCNtupleProducer",
)

process.p = cms.Path(
    process.rpcValTree
)
#process.outPath = cms.EndPath(process.out)




