from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.General.requestName = 'RPCMuonAnalysis'
#config.General.workArea = 'crab_ZeroBias_Run2015D_v3'
config.General.transferOutputs = True
config.General.transferLogs = False

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'run_cfg.py'

config.Data.inputDataset = '/SingleMuon/Run2015B-16Oct2015-v1/MINIAOD'
#config.Data.inputDataset = '/SingleMuon/Run2015C_25ns-05Oct2015-v1/MINIAOD'
#config.Data.inputDataset = '/SingleMuon/Run2015D-05Oct2015-v1/MINIAOD'
#config.Data.inputDataset = '/SingleMuon/Run2015D-PromptReco-v4/MINIAOD'

config.General.workArea = 'crab_'+config.Data.inputDataset.split('/')[2].replace('-', '_')

config.Data.inputDBS = 'global'
config.Data.splitting = 'LumiBased'
config.Data.unitsPerJob = 50
lumiMaskBase = 'https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/certification/Collisions15/13TeV/'
if 'Run2015B' in config.Data.inputDataset:
    config.Data.lumiMask = lumiMaskBase+'Cert_246908-255031_13TeV_PromptReco_Collisions15_50ns_JSON_MuonPhys_v2.txt'
elif 'Run2015' in config.Data.inputDataset:
    config.Data.lumiMask = lumiMaskBase+'Cert_246908-260627_13TeV_PromptReco_Collisions15_25ns_JSON_MuonPhys.txt'
config.Data.runRange = '246908-260627'
config.Data.outLFNDirBase = '/store/user/%s/' % (getUsernameFromSiteDB())
config.Data.publication = False
#config.Data.publishDataName = 'CRAB3_tutorial_May2015_Data_analysis'

config.Site.storageSite = 'T2_CH_CERN'
