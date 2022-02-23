import FWCore.ParameterSet.Config as cms

#--------------------------------------------------------------------------------
# CV: configuration parameters for TallinnTauPiZeroBuilder copied from
#       RecoTauTag/RecoTau/python/RecoTauPiZeroBuilderPlugins_cfi.py
tallinnTauPiZeros = cms.PSet(
    stripCandidatesParticleIds = cms.vint32(2, 4), # e, gamma
    stripEtaAssociationDistanceFunc = cms.PSet(
        function = cms.string("TMath::Min(0.15, TMath::Max(0.05, [0]*TMath::Power(pT, -[1])))"),
        par0 = cms.double(1.97077e-01),
        par1 = cms.double(6.58701e-01)
    ),
    stripPhiAssociationDistanceFunc = cms.PSet(
        function = cms.string("TMath::Min(0.3, TMath::Max(0.05, [0]*TMath::Power(pT, -[1])))"),
        par0 = cms.double(3.52476e-01),
        par1 = cms.double(7.07716e-01)
    ),
    minStripPt = cms.double(2.5),
    verbosity = cms.int32(0)
)
#--------------------------------------------------------------------------------
