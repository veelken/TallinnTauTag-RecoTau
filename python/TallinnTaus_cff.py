import FWCore.ParameterSet.Config as cms

from RecoTauTag.RecoTau.PFRecoTauDiscriminationByHPSSelection_cfi import *
from RecoTauTag.RecoTau.PFRecoTauDiscriminationByLeadingTrackFinding_cfi import *
from RecoTauTag.RecoTau.PFRecoTauDiscriminationByLeadingObjectPtCut_cfi import *
from RecoTauTag.RecoTau.PFRecoTauDiscriminationByIsolation_cfi import *
from TallinnTauTag.RecoTau.TallinnTauProducerDNN_cfi import *
#from TallinnTauTag.RecoTau.TallinnTauProducerGNN_cfi import *
import RecoTauTag.RecoTau.pfTauPrimaryVertexProducer_cfi as _mod
from RecoTauTag.RecoTau.PFRecoTauQualityCuts_cfi import PFTauQualityCuts


tallinnTauDiscriminationByDecayModeFindingOldDMs = hpsSelectionDiscriminator.clone(
    PFTauProducer = 'tallinnTaus',
    decayModes = cms.VPSet(
        decayMode_1Prong0Pi0,
        decayMode_1Prong1Pi0,
        decayMode_1Prong2Pi0,
        decayMode_3Prong0Pi0
    ),
    requireTauChargedHadronsToBeChargedPFCands = True
)
tallinnTauDiscriminationByDecayModeFinding = tallinnTauDiscriminationByDecayModeFindingOldDMs.clone()

tallinnTauDiscriminationByDecayModeFindingNewDMs = hpsSelectionDiscriminator.clone(
    PFTauProducer = 'tallinnTaus',
    decayModes = cms.VPSet(
        decayMode_1Prong0Pi0,
        decayMode_1Prong1Pi0,
        decayMode_1Prong2Pi0,
        decayMode_2Prong0Pi0,
        decayMode_2Prong1Pi0,
        decayMode_3Prong0Pi0,
        decayMode_3Prong1Pi0
    )
)

tallinnTau_requireDecayMode = cms.PSet(
    BooleanOperator = cms.string("and"),
    decayMode = cms.PSet(
        Producer = cms.InputTag('tallinnTauDiscriminationByDecayModeFindingNewDMs'),
        cut = cms.double(0.5)
    )
)

tallinnTauDiscriminationByLeadingTrackFinding = pfRecoTauDiscriminationByLeadingTrackFinding.clone(
    PFTauProducer = 'tallinnTaus',
    UseOnlyChargedHadrons = True
)

tallinnTau_requireLeadTrack = cms.PSet(
    BooleanOperator = cms.string("and"),
    leadTrack = cms.PSet(
        Producer = cms.InputTag('tallinnTauDiscriminationByLeadingTrackFinding'),
        cut = cms.double(0.5)
    )
)

tallinnTauDiscriminationByLeadingTrackPtCut = pfRecoTauDiscriminationByLeadingObjectPtCut.clone(
    PFTauProducer = 'tallinnTaus',
    #Prediscriminants = cms.PSet( BooleanOperator = cms.string("and") ), # use when fix in RecoTauTag/RecoTau/plugins/PFRecoTauDiscriminationByLeadingObjectPtCut.cc not available
    Prediscriminants = tallinnTau_requireLeadTrack.clone(), # else...
    UseOnlyChargedHadrons = True, 
    MinPtLeadingObject = cms.double(5.0)
)

# DeltaBeta correction factor
ak4dBetaCorrection = 0.20

tallinnTauBasicDiscriminators = pfRecoTauDiscriminationByIsolation.clone(
    PFTauProducer = 'tallinnTaus',
    #Prediscriminants = tallinnTau_requireDecayMode.clone(),
    Prediscriminants = tallinnTau_requireLeadTrack.clone(),
    deltaBetaPUTrackPtCutOverride     = True, # Set the boolean = True to override.
    deltaBetaPUTrackPtCutOverride_val = 0.5,  # Set the value for new value.
    customOuterCone = PFRecoTauPFJetInputs.isolationConeSize,
    isoConeSizeForDeltaBeta = 0.8,
    deltaBetaFactor = "%0.4f" % ak4dBetaCorrection,
    qualityCuts = dict(isolationQualityCuts = dict(minTrackHits = 3, minGammaEt = 1.0, minTrackPt = 0.5)),
    IDdefinitions = cms.VPSet(
        cms.PSet(
            IDname = cms.string("ChargedIsoPtSum"),
            ApplyDiscriminationByTrackerIsolation = cms.bool(True),
            storeRawSumPt = cms.bool(True)
        ),
        cms.PSet(
            IDname = cms.string("NeutralIsoPtSum"),
            ApplyDiscriminationByECALIsolation = cms.bool(True),
            storeRawSumPt = cms.bool(True)
        ),
        cms.PSet(
            IDname = cms.string("NeutralIsoPtSumWeight"),
            ApplyDiscriminationByWeightedECALIsolation = cms.bool(True),
            storeRawSumPt = cms.bool(True),
            UseAllPFCandsForWeights = cms.bool(True)
        ),
        cms.PSet(
            IDname = cms.string("TauFootprintCorrection"),
            storeRawFootprintCorrection = cms.bool(True)
        ),
        cms.PSet(
            IDname = cms.string("PhotonPtSumOutsideSignalCone"),
            storeRawPhotonSumPt_outsideSignalCone = cms.bool(True)
        ),
        cms.PSet(
            IDname = cms.string("PUcorrPtSum"),
            applyDeltaBetaCorrection = cms.bool(True),
            storeRawPUsumPt = cms.bool(True)
        ),
        cms.PSet(
            IDname = cms.string("ByRawCombinedIsolationDBSumPtCorr3Hits"),
            ApplyDiscriminationByTrackerIsolation = cms.bool(True),
            ApplyDiscriminationByECALIsolation = cms.bool(True),
            applyDeltaBetaCorrection = cms.bool(True),
            storeRawSumPt = cms.bool(True)
        )
    ),
    IDWPdefinitions = cms.VPSet(
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
)


tallinnTauBasicDiscriminatorsdR03 = tallinnTauBasicDiscriminators.clone(
     deltaBetaFactor = '0.0720', # 0.2*(0.3/0.5)^2
     customOuterCone = 0.3
)

del tallinnTauBasicDiscriminatorsdR03.IDWPdefinitions[-1] # ByPhotonPtSumOutsideSignalCone not defined for dR03
del tallinnTauBasicDiscriminatorsdR03.IDWPdefinitions[-1] # ByLooseChargedIsolation not defined for dR03
for pset in tallinnTauBasicDiscriminatorsdR03.IDdefinitions:
    pset.IDname = pset.IDname.value() + "dR03"
for pset in tallinnTauBasicDiscriminatorsdR03.IDWPdefinitions:
    pset.IDname = pset.IDname.value() + "dR03"
    pset.referenceRawIDNames = [name + "dR03" for name in pset.referenceRawIDNames.value()]
tallinnTauBasicDiscriminatorsdR03Task = cms.Task(
    tallinnTauBasicDiscriminatorsdR03
)

tallinnTauPrimaryVertexProducer = _mod.pfTauPrimaryVertexProducer.clone(
    #Algorithm: 0 - use tau-jet vertex, 1 - use vertex[0]
    qualityCuts = PFTauQualityCuts,
    discriminators = cms.VPSet(
        cms.PSet(
            discriminator = cms.InputTag('tallinnTauDiscriminationByDecayModeFindingNewDMs'),
            selectionCut = cms.double(0.5)
        )
    ),
    PFTauTag = cms.InputTag('tallinnTaus')
)

tallinnTauSecondaryVertexProducer = cms.EDProducer('PFTauSecondaryVertexProducer',
  PFTauTag = cms.InputTag('tallinnTaus'),
  mightGet = cms.optional.untracked.vstring
)
tallinnTauTransverseImpactParameters = cms.EDProducer('PFTauTransverseImpactParameters',
  PFTauPVATag = cms.InputTag('tallinnTauPrimaryVertexProducer'),
  useFullCalculation = cms.bool(False),
  PFTauTag = cms.InputTag('tallinnTaus'),
  PFTauSVATag = cms.InputTag('tallinnTauSecondaryVertexProducer'),
  mightGet = cms.optional.untracked.vstring
)

if tallinnTaus.mode == cms.string("regression"):
    tallinnTauSequence = cms.Sequence(
        tallinnTaus
        + tallinnTauDiscriminationByDecayModeFindingOldDMs
        + tallinnTauDiscriminationByDecayModeFinding
        + tallinnTauDiscriminationByDecayModeFindingNewDMs
        + tallinnTauDiscriminationByLeadingTrackFinding
        + tallinnTauDiscriminationByLeadingTrackPtCut
        + tallinnTauBasicDiscriminators
        + tallinnTauBasicDiscriminatorsdR03
    )
elif tallinnTaus.mode == cms.string("classification"):
        tallinnTauSequence = cms.Sequence(
        tallinnTaus
        + tallinnTauDiscriminationByDecayModeFinding
        + tallinnTauDiscriminationByDecayModeFindingNewDMs
        + tallinnTauDiscriminationByLeadingTrackFinding
        + tallinnTauDiscriminationByLeadingTrackPtCut
        + tallinnTauBasicDiscriminators
        + tallinnTauBasicDiscriminatorsdR03
        + tallinnTauPrimaryVertexProducer
        + tallinnTauSecondaryVertexProducer
        + tallinnTauTransverseImpactParameters
    )
