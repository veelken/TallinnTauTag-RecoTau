
import FWCore.ParameterSet.Config as cms
from RecoTauTag.RecoTau.PFRecoTauQualityCuts_cfi import PFTauQualityCuts
from RecoTauTag.RecoTau.PFRecoTauPFJetInputs_cfi import PFRecoTauPFJetInputs
from TallinnTauTag.RecoTau.TallinnTauPiZeroBuilder_cfi import tallinnTauPiZeros
'''

Configuration for DNN/GNN-based PFTau producer developed by Tallinn group.

Author: Christian Veelken, Tallinn


'''

tallinnTaus = cms.EDProducer("TallinnTauProducer",
    pfJetSrc = PFRecoTauPFJetInputs.inputJetCollection,
    minJetPt = PFRecoTauPFJetInputs.minJetPt,
    maxJetAbsEta = PFRecoTauPFJetInputs.maxJetAbsEta,
    pfCandSrc = cms.InputTag("particleFlow"),
    jetInputs = cms.vstring("pt", "eta", "phi", "mass", "numConstituents"),
    pfCandInputs = cms.vstring("pt", "eta", "phi", "particleId", "dz", "dxy"),
    maxNumPFCands = cms.uint32(25),
    graphs = cms.VPSet(
        cms.PSet(
            inputFile = cms.string("TallinnTauTag/RecoTau/data/dnn_2020Feb21.pb"),
            graphName = cms.string("")
        )
    ),
    signalMinPFEnFrac = cms.double(0.),
    isolationMinPFEnFrac = cms.double(0.),
    isolationConeSize = PFRecoTauPFJetInputs.isolationConeSize,
    qualityCuts = PFTauQualityCuts,
    piZeros = tallinnTauPiZeros,
    verbosity = cms.int32(0)
)
