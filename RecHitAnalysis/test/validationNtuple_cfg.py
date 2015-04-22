import FWCore.ParameterSet.Config as cms

process = cms.Process("RPCRecHitValidation")

### standard includes
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("Configuration.StandardSequences.Services_cff")
process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("Configuration.StandardSequences.RawToDigi_cff")

process.MessageLogger.cerr.FwkReport.reportEvery = 1000

### conditions
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#from Configuration.PyReleaseValidation.autoCond import autoCond
#process.GlobalTag.globaltag = autoCond['startup']
process.GlobalTag.globaltag = 'GR_R_74_V8::All'
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

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
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/02040732-DFA7-E211-8C20-E0CB4E29C4B7.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/04009F1C-C8A7-E211-AA4D-E0CB4E1A11A7.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/0857F9FF-BBA7-E211-8433-E0CB4E55363D.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/0A02695F-90A7-E211-896E-001E4F3F3556.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/0C330064-A8A7-E211-BC39-00259073E45E.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/0C7163E9-9FA7-E211-A0D6-00259073E382.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/1010C4E3-A1A7-E211-B288-00259074AE9A.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/128E9086-9EA7-E211-BEF9-001EC9D4A1FD.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/12927D85-F4A7-E211-8ECE-E0CB4E29C4B7.root',
    '/store/data/Run2012D/SingleMu/RAW-RECO/ZMu-22Jan2013-v1/10000/12B45D3F-A7A7-E211-A827-00259073E382.root',
])

process.rpcValTree = cms.EDAnalyzer("RPCNtupleProducer",
)

process.p = cms.Path(
    process.rpcValTree
)
#process.outPath = cms.EndPath(process.out)




