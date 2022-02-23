import FWCore.ParameterSet.Config as cms
 
#-------------------------------------------------------------------------------
# CV: configuration for pat::Tau production copied from
#       PhysicsTools/PatAlgos/python/producersLayer1/tauProducer_cff.py
#    (and mofified)

from PhysicsTools.JetMCAlgos.TauGenJets_cfi import *
from PhysicsTools.JetMCAlgos.TauGenJetsDecayModeSelectorAllHadrons_cfi import *
from PhysicsTools.PatAlgos.mcMatchLayer0.tauMatch_cfi import *
tallinnTauMatch = tauMatch.clone(
    src = 'tallinnTaus'
)
tallinnTauGenJetMatch = tauGenJetMatch.clone(
    src = 'tallinnTaus'
)

# produce object
from PhysicsTools.PatAlgos.producersLayer1.tauProducer_cfi import *
patTallinnTaus = patTaus.clone(
    tauSource = 'tallinnTaus',
    tauTransverseImpactParameterSource = '',
    genParticleMatch = 'tallinnTauMatch',
    genJetMatch = 'tallinnTauGenJetMatch'
)

# add tau ID discriminators
singleID(patTallinnTaus.tauIDSources, 'tallinnTauDiscriminationByDecayModeFindingNewDMs', "decayModeFindingNewDMs")
containerID(patTallinnTaus.tauIDSources, 'tallinnTauBasicDiscriminators', "IDdefinitions", [
    [ 'chargedIsoPtSum', "ChargedIsoPtSum" ],
    [ 'neutralIsoPtSum', "NeutralIsoPtSum" ],
    [ 'puCorrPtSum', "PUcorrPtSum" ],
    [ 'neutralIsoPtSumWeight', "NeutralIsoPtSumWeight" ],
    [ 'footprintCorrection', "TauFootprintCorrection" ],
    [ 'photonPtSumOutsideSignalCone', "PhotonPtSumOutsideSignalCone" ],
    [ 'byCombinedIsolationDeltaBetaCorrRaw3Hits', "ByRawCombinedIsolationDBSumPtCorr3Hits" ]
])
containerID(patTallinnTaus.tauIDSources, 'tallinnTauBasicDiscriminators', "IDWPdefinitions", [
    [ 'byLooseCombinedIsolationDeltaBetaCorr3Hits', "ByLooseCombinedIsolationDBSumPtCorr3Hits" ],
    [ 'byMediumCombinedIsolationDeltaBetaCorr3Hits', "ByMediumCombinedIsolationDBSumPtCorr3Hits" ],
    [ 'byTightCombinedIsolationDeltaBetaCorr3Hits', "ByTightCombinedIsolationDBSumPtCorr3Hits" ],
    [ 'byLooseChargedIsolation', "ByLooseChargedIsolation" ],
    [ 'byMediumChargedIsolation', "ByMediumChargedIsolation" ],
    [ 'byTightChargedIsolation', "ByTightChargedIsolation" ],
    [ 'byPhotonPtSumOutsideSignalCone', "ByPhotonPtSumOutsideSignalCone" ]
])

patTallinnTauSequence = cms.Sequence(
  tallinnTauMatch
 + tauGenJets
 + tauGenJetsSelectorAllHadrons
 + tallinnTauGenJetMatch
 + patTallinnTaus
)
#-------------------------------------------------------------------------------
