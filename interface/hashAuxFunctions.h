#ifndef TallinnTauTag_RecoTau_hashAuxFunctions_h
#define TallinnTauTag_RecoTau_hashAuxFunctions_h

#include "DataFormats/Candidate/interface/Candidate.h"               // reco::Candidate::LorentzVector
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h" // reco::PFCandidate::ParticleType 

#include <string> // std::string

namespace reco 
{
  namespace tau 
  {
    std::string
    getHash_jet(const reco::Candidate::LorentzVector& p4);

    std::string
    getHash_pfCand(const reco::Candidate::LorentzVector& p4, reco::PFCandidate::ParticleType particleId, int charge);
  }
}

#endif
