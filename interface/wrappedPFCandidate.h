#ifndef TallinnTauTag_RecoTau_wrappedPFCandidate_h
#define TallinnTauTag_RecoTau_wrappedPFCandidate_h

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h" // reco::PFCandidate, reco::PFCandidatePtr, reco::PFCandidate::ParticleType
#include "DataFormats/Candidate/interface/Candidate.h"               // reco::Candidate::LorentzVector

#include <vector>  // std::vector

namespace reco
{
  class wrappedPFCandidate
  {
   public:
    explicit wrappedPFCandidate(const reco::PFCandidate& pfCand, const reco::PFCandidatePtr& pfCandPtr, double enFrac);
    ~wrappedPFCandidate();

    const reco::Candidate::LorentzVector&
    p4() const;

    double
    energy() const;

    double
    et() const;

    double
    pt() const;

    double
    eta() const;

    double
    phi() const;

    int
    charge() const;

    reco::PFCandidate::ParticleType
    particleId() const;

    const reco::PFCandidate& 
    pfCand() const;

    const reco::PFCandidatePtr& 
    pfCandPtr() const;
   
    double
    enFrac() const;

   private:
    reco::PFCandidate pfCand_;
    reco::PFCandidatePtr pfCandPtr_;
    double enFrac_;
  };

  typedef std::vector<wrappedPFCandidate> wrappedPFCandidateCollection;
}

#endif
