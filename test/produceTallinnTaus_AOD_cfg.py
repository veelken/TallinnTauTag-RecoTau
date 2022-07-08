import FWCore.ParameterSet.Config as cms

process = cms.Process("produceTallinnTaus")

process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.default.reportEvery = 100
process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(50000)
    ##input = cms.untracked.int32(10)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'file:/hdfs/local/tolange/step3/train/0/step3_htt.root'
    ),
    dropDescendantsOfDroppedBranches = cms.untracked.bool(True),
    inputCommands=cms.untracked.vstring(
        'keep *',
        'drop *_hpsPFTau*_*_*'
    )
)

inputFilePath = "/hdfs/local/tolange/step3/val/"
mode = "dnn"
#mode = "gnn"

##inputFilePath = "$inputFilePath"
##mode = "$mode"

#--------------------------------------------------------------------------------
# set input files

import os
import re

inputFile_regex = r"[a-zA-Z0-9-_]+.root"
inputFile_matcher = re.compile(inputFile_regex)

def getInputFileNames(inputFilePath):
    inputFileNames = []
    files = os.listdir(inputFilePath)
    for file in files:
        if os.path.isdir(os.path.join(inputFilePath, file)):
            inputFileNames.extend(getInputFileNames(os.path.join(inputFilePath, file)))
        else:
            # check if name of inputFile matches regular expression
            if inputFile_matcher.match(file):
                if not("inMINIAODSIM" in file):
                    inputFileNames.append("file:%s" % os.path.join(inputFilePath, file))
    return inputFileNames

print("Searching for input files in path = '%s'" % inputFilePath)
inputFileNames = getInputFileNames(inputFilePath)
print("Found %i input files." % len(inputFileNames))
process.source.fileNames = cms.untracked.vstring(inputFileNames)
#--------------------------------------------------------------------------------

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')

process.productionSequence = cms.Sequence()

##process.dumpEventContent = cms.EDAnalyzer("EventContentAnalyzer") 
##process.productionSequence += process.dumpEventContent

#--------------------------------------------------------------------------------
# CV: run HPS tau reconstruction with charged isolation tau ID discriminators added
#     and store taus in pat::Tau format
process.load("RecoTauTag.Configuration.RecoPFTauTag_cff")
from RecoTauTag.RecoTau.PFRecoTauDiscriminationByLeadingTrackFinding_cfi import pfRecoTauDiscriminationByLeadingTrackFinding 
process.hpsPFTauDiscriminationByLeadingTrackFinding = pfRecoTauDiscriminationByLeadingTrackFinding.clone(
    PFTauProducer = 'hpsPFTauProducer',
    UseOnlyChargedHadrons = True
)
hpsPFTau_requireLeadTrack = cms.PSet(
    BooleanOperator = cms.string("and"),
    leadTrack = cms.PSet(
        Producer = cms.InputTag('hpsPFTauDiscriminationByLeadingTrackFinding'),
        cut = cms.double(0.5)
    )
)
from RecoTauTag.RecoTau.PFRecoTauDiscriminationByLeadingObjectPtCut_cfi import pfRecoTauDiscriminationByLeadingObjectPtCut
process.hpsPFTauDiscriminationByLeadingTrackPtCut = pfRecoTauDiscriminationByLeadingObjectPtCut.clone(
    PFTauProducer = 'hpsPFTauProducer',
    #Prediscriminants = cms.PSet( BooleanOperator = cms.string("and") ), # use when fix in RecoTauTag/RecoTau/plugins/PFRecoTauDiscriminationByLeadingObjectPtCut.cc not available
    Prediscriminants = hpsPFTau_requireLeadTrack.clone(), # else...
    UseOnlyChargedHadrons = True, 
    MinPtLeadingObject = cms.double(5.0)
)
process.hpsPFTauBasicDiscriminators.Prediscriminants = hpsPFTau_requireLeadTrack.clone(),
process.hpsPFTauBasicDiscriminators.IDWPdefinitions = cms.VPSet(
    cms.PSet(
        IDname = cms.string("ByLooseCombinedIsolationDBSumPtCorr3Hits"),
        referenceRawIDNames = cms.vstring("ByRawCombinedIsolationDBSumPtCorr3Hits", "PhotonPtSumOutsideSignalCone"),
        maximumAbsoluteValues = cms.vdouble(2.5, 1.e+9),
        maximumRelativeValues = cms.vdouble(-1.0, 0.10)
    ),
    cms.PSet(
        IDname = cms.string("ByMediumCombinedIsolationDBSumPtCorr3Hits"),
        referenceRawIDNames = cms.vstring("ByRawCombinedIsolationDBSumPtCorr3Hits", "PhotonPtSumOutsideSignalCone"),
        maximumAbsoluteValues = cms.vdouble(1.5, 1.e+9),
        maximumRelativeValues = cms.vdouble(-1.0, 0.10)
    ),
    cms.PSet(
        IDname = cms.string("ByTightCombinedIsolationDBSumPtCorr3Hits"),
        referenceRawIDNames = cms.vstring("ByRawCombinedIsolationDBSumPtCorr3Hits", "PhotonPtSumOutsideSignalCone"),
        maximumAbsoluteValues = cms.vdouble(0.8, 1.e+9),
        maximumRelativeValues = cms.vdouble(-1.0, 0.10)
    ),
    cms.PSet(
        IDname = cms.string("ByLooseChargedIsolation"),
        referenceRawIDNames = cms.vstring("ChargedIsoPtSum"),
        maximumAbsoluteValues = cms.vdouble(2.5)
    ),
    cms.PSet(
        IDname = cms.string("ByMediumChargedIsolation"),
        referenceRawIDNames = cms.vstring("ChargedIsoPtSum"),
        maximumAbsoluteValues = cms.vdouble(1.5)
    ),
    cms.PSet(
        IDname = cms.string("ByTightChargedIsolation"),
        referenceRawIDNames = cms.vstring("ChargedIsoPtSum"),
        maximumAbsoluteValues = cms.vdouble(0.8)
    ),
    cms.PSet(
        IDname = cms.string("ByPhotonPtSumOutsideSignalCone"),
        referenceRawIDNames = cms.vstring("PhotonPtSumOutsideSignalCone"),
        maximumRelativeValues = cms.vdouble(0.10)
    )
)
process.productionSequence += process.PFTau
process.productionSequence += process.hpsPFTauDiscriminationByLeadingTrackFinding
process.productionSequence += process.hpsPFTauDiscriminationByLeadingTrackPtCut
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# CV: run Tallinn tau reconstruction
#     and store taus in pat::Tau format
process.load("TallinnTauTag.RecoTau.TallinnTaus_cff")
if mode == "dnn":
    from TallinnTauTag.RecoTau.TallinnTauProducerDNN_cfi import tallinnTaus as tallinnTausDNN
    process.tallinnTaus = tallinnTausDNN
elif mode == "gnn":
    from TallinnTauTag.RecoTau.TallinnTauProducerGNN_cfi import tallinnTaus as tallinnTausGNN
    process.tallinnTaus = tallinnTausGNN
else:
    raise ValueError("Invalid configuration parameter 'mode' = '%s'!!" % mode) 
#process.tallinnTaus.mode = cms.string('regression')
process.productionSequence += process.tallinnTauSequence

#--------------------------------------------------------------------------------

process.p = cms.Path(process.productionSequence)

from Configuration.EventContent.EventContent_cff import AODSIMEventContent
myOutputCommands = AODSIMEventContent.outputCommands
myOutputCommands.append("keep *_hpsPFTau*_*_*")
myOutputCommands.append("keep *_tallinnTau*_*_*")
myOutputCommands.append("keep *_caloStage2Digis_*_*")
myOutputCommands.append("keep *_gmtStage2Digis_*_*")
myOutputCommands.append("keep *_gtStage2Digis_*_*")
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('produceTallinnTaus_AODSIM.root'),
    outputCommands = myOutputCommands
)
process.q = cms.EndPath(process.out)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

##dump_file = open('dump.py','w')
##dump_file.write(process.dumpPython())
