import FWCore.ParameterSet.Config as cms

process = cms.Process("Ana")

### standard includes
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.Services_cff")
#process.load("Configuration.StandardSequences.GeometryDB_cff")
#process.load("Configuration.StandardSequences.MagneticField_38T_cff")
#process.load("RecoMuon.TrackingTools.MuonServiceProxy_cff")

process.MessageLogger.cerr.FwkReport.reportEvery = 1000

### conditions
#process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff")
#from Configuration.AlCa.autoCond import autoCond
#process.GlobalTag.globaltag = autoCond['startup']
#process.GlobalTag.globaltag = 'GR_R_74_V8::All'
process.GlobalTag.globaltag = '74X_dataRun2_Prompt_v2'
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    allowUnscheduled = cms.untracked.bool(True),
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(),
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("ntuple.root")
)

process.source.fileNames.extend([
    '/store/relval/CMSSW_7_4_15/RelValZMM_13/MINIAODSIM/PU25ns_74X_mcRun2_asymptotic_v2-v1/00000/10FF6E32-3C72-E511-87AD-0025905A60B4.root',
    '/store/relval/CMSSW_7_4_15/RelValZMM_13/MINIAODSIM/PU25ns_74X_mcRun2_asymptotic_v2-v1/00000/E03C4936-3C72-E511-ADF4-0025905A60BE.root',
])

process.rpcMuon = cms.EDAnalyzer("RPCMuonOptAnalyzer",
    vertex = cms.InputTag("offlineSlimmedPrimaryVertices"),
    muon = cms.InputTag("slimmedMuons"),
    triggerObject = cms.InputTag("selectedPatTrigger"),
    hltPrefixes = cms.vstring("HLT_IsoMu22", "HLT_IsoMu20",),
    minPt = cms.double(4),
    maxEta = cms.double(2.5),
)

process.p = cms.Path(
    process.rpcMuon
)
