import FWCore.ParameterSet.Config as cms

process = cms.Process("DQM2")
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.load('Configuration.StandardSequences.EDMtoMEAtRunEnd_cff')

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1))
process.MessageLogger.cerr.FwkReport.reportEvery = 10000

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/data/Run2012D/SingleMu/DQM/22Jan2013-v1/10000/00D477E9-5BA8-E211-BD14-00261834B5C1.root',
    ),
)

process.blacklistExtractor = cms.EDAnalyzer("BlacklistExtractor",
)

process.p = cms.Path(
    process.EDMtoME*process.blacklistExtractor
)

