#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidate.h"

using namespace reco;

wrappedPFCandidate::wrappedPFCandidate(const reco::PFCandidate& pfCand, const reco::PFCandidatePtr& pfCandPtr, double enFrac)
  : pfCand_(pfCand)
  , pfCandPtr_(pfCandPtr)
  , enFrac_(enFrac)
{}

wrappedPFCandidate::~wrappedPFCandidate()
{}

const reco::Candidate::LorentzVector&
wrappedPFCandidate::p4() const
{
  return pfCand_.p4();
}

double
wrappedPFCandidate::energy() const
{
  return pfCand_.energy();
}

double
wrappedPFCandidate::et() const
{
  return pfCand_.et();
}

double
wrappedPFCandidate::pt() const
{
  return pfCand_.pt();
}

double
wrappedPFCandidate::eta() const
{
  return pfCand_.eta();
}

double
wrappedPFCandidate::phi() const
{
  return pfCand_.phi();
}

int
wrappedPFCandidate::charge() const
{
  return pfCand_.charge();
}

reco::PFCandidate::ParticleType
wrappedPFCandidate::particleId() const
{
  return pfCand_.particleId();
}

const reco::PFCandidate& 
wrappedPFCandidate::pfCand() const
{
  return pfCand_;
}

const reco::PFCandidatePtr& 
wrappedPFCandidate::pfCandPtr() const
{
  return pfCandPtr_;
}
   
double
wrappedPFCandidate::enFrac() const
{
  return enFrac_;
}
