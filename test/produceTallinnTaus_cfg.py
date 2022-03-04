import FWCore.ParameterSet.Config as cms

process = cms.Process("produceTallinnTaus")

process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    ##input = cms.untracked.int32(-1)
    input = cms.untracked.int32(10)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        ##'file:/hdfs/local/tolange/PFMatching/singleTau_mini_wPU.root'
        'file:/hdfs/local/tolange/step3/0/step3_htt.root'
    )
)

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')

process.productionSequence = cms.Sequence()

#process.dumpEventContent = cms.EDAnalyzer("EventContentAnalyzer") 
#process.productionSequence += process.dumpEventContent

#--------------------------------------------------------------------------------
# CV: run HPS tau reconstruction with charged isolation tau ID discriminators added
#     and store taus in pat::Tau format

process.load("RecoTauTag.Configuration.HPSPFTaus_cff")
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
process.productionSequence += process.produceAndDiscriminateHPSPFTaus

process.load("PhysicsTools.PatAlgos.producersLayer1.tauProducer_cff")
from PhysicsTools.PatAlgos.producersLayer1.tauProducer_cfi import singleID, containerID
process.patTaus.tauIDSources = cms.PSet()
singleID(process.patTaus.tauIDSources, 'hpsPFTauDiscriminationByDecayModeFinding', "decayModeFinding")
singleID(process.patTaus.tauIDSources, 'hpsPFTauDiscriminationByDecayModeFindingNewDMs', "decayModeFindingNewDMs")
containerID(process.patTaus.tauIDSources, "hpsPFTauBasicDiscriminators", "IDWPdefinitions", [
    [ 'byLooseCombinedIsolationDeltaBetaCorr3Hits', "ByLooseCombinedIsolationDBSumPtCorr3Hits" ],
    [ 'byMediumCombinedIsolationDeltaBetaCorr3Hits', "ByMediumCombinedIsolationDBSumPtCorr3Hits" ],
    [ 'byTightCombinedIsolationDeltaBetaCorr3Hits', "ByTightCombinedIsolationDBSumPtCorr3Hits" ],
    [ 'byLooseChargedIsolation', "ByLooseChargedIsolation" ],
    [ 'byMediumChargedIsolation', "ByMediumChargedIsolation" ],
    [ 'byTightChargedIsolation', "ByTightChargedIsolation" ],
    [ 'byPhotonPtSumOutsideSignalCone', "ByPhotonPtSumOutsideSignalCone"]
])
process.productionSequence += process.makePatTaus
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# CV: run Tallinn tau reconstruction
#     and store taus in pat::Tau format

process.load("TallinnTauTag.RecoTau.TallinnTaus_cff")
process.productionSequence += process.tallinnTauSequence

process.load("TallinnTauTag.RecoTau.patTallinnTaus_cff")
process.productionSequence += process.patTallinnTauSequence
#--------------------------------------------------------------------------------

process.p = cms.Path(process.productionSequence)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('produceTallinnTaus.root'),
    outputCommands = cms.untracked.vstring(
        ##"keep *_*_*_*"
        "drop *_*_*_*",
        "keep *_tauGenJets_*_*",
        "keep *_tauGenJetsSelectorAllHadrons_*_*",
        "keep *_ak4PFJets_*_*",
        "keep *_particleFlow_*_*",
        "keep *_patTaus_*_*",
        "keep *_patTallinnTaus_*_*"
    )
)
process.q = cms.EndPath(process.out)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

##dump_file = open('dump.py','w')
##dump_file.write(process.dumpPython())
