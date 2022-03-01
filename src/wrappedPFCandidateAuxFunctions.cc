#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidateAuxFunctions.h"

bool
reco::tau::isHigherPt(const reco::wrappedPFCandidate& pfCand1, const reco::wrappedPFCandidate& pfCand2)
{
  return pfCand1.pt() > pfCand2.pt();
}

reco::wrappedPFCandidateCollection
reco::tau::getPFCands_of_type(const reco::wrappedPFCandidateCollection& pfCands, const std::vector<int>& particleIds)
{
  reco::wrappedPFCandidateCollection pfCands_of_type;
  for ( auto const& pfCand : pfCands )
  {
    bool isPFCand_of_type = false;
    for ( auto const& particleId : particleIds )
    {
      if ( pfCand.particleId() == particleId )
      {
        isPFCand_of_type = true;
        break;
      }
    }
    if ( isPFCand_of_type ) pfCands_of_type.push_back(pfCand);
  }
  return pfCands_of_type;
}

reco::Candidate::LorentzVector
reco::tau::getSumP4(const reco::wrappedPFCandidateCollection& pfCands)
{
  reco::Candidate::LorentzVector sumP4;
  for ( auto const& pfCand : pfCands )
  {
    sumP4 += pfCand.p4();
  }
  return sumP4;
}
