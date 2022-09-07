import glob, os

# inputFilesQCD="/hdfs/cms/store/mc/Run3Winter21DRMiniAOD/QCD_Pt-15to7000_TuneCUETP8M1_Flat_14TeV-pythia8/AODSIM/112X_mcRun3_TL180fb_Th180fb_realistic_v1-v2/*/*.root"
# qcdfiles = glob.glob(inputFilesQCD)
# qcdConfigPath = "/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/qcdConfigs/"
# if not os.path.exists(qcdConfigPath): os.makedirs(qcdConfigPath)
# for nf, f in enumerate(qcdfiles):
#     JOB = 'qcd_' + str(nf)
#     FILEPATH = '"file:'+f.replace('/','\/') + '"'
#     outfile = open(qcdConfigPath + JOB + "_slurm","w")
#     template = open("/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/produceMiniAODforfile.sh")
#     for l in template:
#         outfile.write(l.replace('JOB',JOB).replace('FILEPATH',FILEPATH).replace('CONF',qcdConfigPath))
#     outfile.close()
#     os.system('sbatch ' + qcdConfigPath + JOB + "_slurm")


# inputFilesHTT="/hdfs/cms/store/mc/Run3Summer21DRPremix/GluGluHToTauTau_M-125_TuneCP5_14TeV-powheg-pythia8/AODSIM/120X_mcRun3_2021_realistic_v6-v2/*/*.root"
# httfiles = glob.glob(inputFilesHTT)
# httConfigPath = "/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/httConfigs/"
# if not os.path.exists(httConfigPath): os.makedirs(httConfigPath)
# for nf, f in enumerate(httfiles):
#     JOB = 'htt_' + str(nf)
#     FILEPATH = '"file:'+f.replace('/','\/') + '"'
#     outfile = open(httConfigPath + JOB + "_slurm","w")
#     template = open("/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/produceMiniAODforfile.sh")
#     for l in template:
#         outfile.write(l.replace('JOB',JOB).replace('FILEPATH',FILEPATH).replace('CONF',httConfigPath))
#     outfile.close()
#     os.system('sbatch ' + httConfigPath + JOB + "_slurm")


# inputFilesDYEE="/hdfs/cms/store/mc/Run3Winter21DRMiniAOD/ZToEE_TuneCUETP8M1_14TeV-pythia8/AODSIM/112X_mcRun3_TL180fb_Th180fb_realistic_v1-v2/*/*.root"
# dyEEfiles = glob.glob(inputFilesDYEE)
# dyEEConfigPath = "/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/dyEEConfigs/"
# if not os.path.exists(dyEEConfigPath): os.makedirs(dyEEConfigPath)
# for nf, f in enumerate(dyEEfiles):
#     JOB = 'dyEE_' + str(nf)
#     FILEPATH = '"file:'+f.replace('/','\/') + '"'
#     outfile = open(dyEEConfigPath + JOB + "_slurm","w")
#     template = open("/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/produceMiniAODforfile.sh")
#     for l in template:
#         outfile.write(l.replace('JOB',JOB).replace('FILEPATH',FILEPATH).replace('CONF',dyEEConfigPath))
#     outfile.close()
#     os.system('sbatch ' + dyEEConfigPath + JOB + "_slurm")


#need newer cmssw!
inputFilesDYMUMU="/hdfs/cms/store/mc/Run3Winter22DRPremix/DYToMuMu_M-20_TuneCP5_13p6TeV-pythia8/AODSIM/122X_mcRun3_2021_realistic_v9-v1/*/*.root"
dyMuMufiles = glob.glob(inputFilesDYMUMU)
dyMuMuConfigPath = "/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/dyMuMuConfigs/"
if not os.path.exists(dyMuMuConfigPath): os.makedirs(dyMuMuConfigPath)
for nf, f in enumerate(dyMuMufiles):
    if nf>0: continue
    JOB = 'dyMuMu_' + str(nf)
    FILEPATH = '"file:'+f.replace('/','\/') + '"'
    outfile = open(dyMuMuConfigPath + JOB + "_slurm","w")
    template = open("/home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/produceMiniAODforfile.sh")
    for l in template:
        outfile.write(l.replace('JOB',JOB).replace('FILEPATH',FILEPATH).replace('CONF',dyMuMuConfigPath))
    outfile.close()
    os.system('sbatch ' + dyMuMuConfigPath + JOB + "_slurm")
