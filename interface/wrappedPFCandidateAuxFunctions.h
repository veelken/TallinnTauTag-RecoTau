#ifndef TallinnTauTag_RecoTau_wrappedPFCandidateAuxFunctions_h
#define TallinnTauTag_RecoTau_wrappedPFCandidateAuxFunctions_h

#include "DataFormats/Candidate/interface/Candidate.h"          // reco::Candidate::LorentzVector 

#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidate.h" // reco::wrappedPFCandidate, reco::wrappedPFCandidateCollection

#include <vector>                                               // std::vector

namespace reco
{
  namespace tau 
  {
    bool
    isHigherPt(const reco::wrappedPFCandidate& pfCand1, const reco::wrappedPFCandidate& pfCand2);

    reco::wrappedPFCandidateCollection
    getPFCands_of_type(const reco::wrappedPFCandidateCollection& pfCands, const std::vector<int>& particleIds);

    reco::Candidate::LorentzVector
    getSumP4(const reco::wrappedPFCandidateCollection& pfCands);
  }
}

#endif
