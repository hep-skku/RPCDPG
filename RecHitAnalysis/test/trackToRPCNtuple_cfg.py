import FWCore.ParameterSet.Config as cms

process = cms.Process("RPCRecHitValidation")

### standard includes
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.Services_cff")
process.load("Configuration.StandardSequences.GeometryDB_cff")
#process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("RecoMuon.TrackingTools.MuonServiceProxy_cff")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("TrackingTools.TrackAssociator.DetIdAssociatorESProducer_cff")

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
    wantSummary = cms.untracked.bool(True)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(),
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("ntuple.root")
)

process.source.fileNames.extend([
    '/store/relval/CMSSW_7_4_15/RelValZMM_13/GEN-SIM-RECO/PU25ns_74X_mcRun2_asymptotic_v2-v1/00000/18B13146-3872-E511-A382-00261894394D.root',
    '/store/relval/CMSSW_7_4_15/RelValZMM_13/GEN-SIM-RECO/PU25ns_74X_mcRun2_asymptotic_v2-v1/00000/4C8C144F-3872-E511-9A4F-0025905A6132.root',
    '/store/relval/CMSSW_7_4_15/RelValZMM_13/GEN-SIM-RECO/PU25ns_74X_mcRun2_asymptotic_v2-v1/00000/742E923A-3C72-E511-B7CB-0025905B857C.root',
    '/store/relval/CMSSW_7_4_15/RelValZMM_13/GEN-SIM-RECO/PU25ns_74X_mcRun2_asymptotic_v2-v1/00000/A02529DD-3972-E511-B68B-002618943980.root',
    '/store/relval/CMSSW_7_4_15/RelValZMM_13/GEN-SIM-RECO/PU25ns_74X_mcRun2_asymptotic_v2-v1/00000/D43B7733-3C72-E511-B1F2-0025905A60B4.root',
])

process.trackRPC = cms.EDAnalyzer("TrackToRPCNtupleMaker",
    minPt = cms.double(4),
    maxEta = cms.double(2.5),
)
"""
    ## TA parameters from TrackingTools/TrackAssociator/python/default_cfi.py
    TrackAssociatorParameters = cms.PSet(
      useMuon = cms.bool(True),
      useCalo = cms.bool(False),
      useEcal = cms.bool(False),
      useHO = cms.bool(False),
      usePreshower = cms.bool(False),
      truthMatch = cms.bool(False),
      useHcal = cms.bool(False),
      accountForTrajectoryChangeCalo = cms.bool(True),
      propagateAllDirections = cms.bool(True),

      muonMaxDistanceX = cms.double(5.0),
      muonMaxDistanceY = cms.double(5.0),
      muonMaxDistanceSigmaX = cms.double(0.0),
      muonMaxDistanceSigmaY = cms.double(0.0),
      trajectoryUncertaintyTolerance = cms.double(-1.0),

      CSCSegmentCollectionLabel = cms.InputTag("cscSegments"),
      CaloTowerCollectionLabel = cms.InputTag("towerMaker"),

      dRMuon = cms.double(9999.0),
      dREcal = cms.double(9999.0),
      dRHcal = cms.double(9999.0),
      dRMuonPreselection = cms.double(0.2),
      dREcalPreselection = cms.double(0.05),
      dRHcalPreselection = cms.double(0.2),
      dRPreshowerPreselection = cms.double(0.2),
      HORecHitCollectionLabel = cms.InputTag("horeco"),

      DTRecSegment4DCollectionLabel = cms.InputTag("dt4DSegments"),
      EERecHitCollectionLabel = cms.InputTag("ecalRecHit","EcalRecHitsEE"),
      EBRecHitCollectionLabel = cms.InputTag("ecalRecHit","EcalRecHitsEB"),
      HBHERecHitCollectionLabel = cms.InputTag("hbhereco"),
    ),
)
"""

process.p = cms.Path(
    process.trackRPC
)
