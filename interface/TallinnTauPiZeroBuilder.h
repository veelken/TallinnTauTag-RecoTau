#ifndef TallinnTauTag_RecoTau_TallinnTauPiZeroBuilder_h
#define TallinnTauTag_RecoTau_TallinnTauPiZeroBuilder_h

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"      // edm::ParameterSetDescription 

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"    // reco::PFCandidate
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h" // reco::PFCandidatePtr
#include "DataFormats/VertexReco/interface/Vertex.h"                    // reco::Vertex::Point

#include "DataFormats/TauReco/interface/RecoTauPiZero.h"                // reco::RecoTauPiZero
#include "DataFormats/TauReco/interface/RecoTauPiZeroFwd.h"             // reco::RecoTauPiZeroCollection

#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidate.h"         // reco::wrappedPFCandidate, reco::wrappedPFCandidateCollection

#include "TFormula.h"

namespace reco 
{
  namespace tau 
  {
    class TallinnTauPiZeroBuilder
    {
     public:
      explicit TallinnTauPiZeroBuilder(const edm::ParameterSet& cfg);
      ~TallinnTauPiZeroBuilder();

      reco::RecoTauPiZeroCollection 
      operator()(const reco::wrappedPFCandidateCollection& pfCands,
                 const reco::Vertex::Point& primaryVertexPos);

      static void 
      fillDescriptions(edm::ParameterSetDescription& desc);

     private:
      std::vector<int> inputParticleIds_;
      std::unique_ptr<const TFormula> etaAssociationDistance_;
      std::unique_ptr<const TFormula> phiAssociationDistance_;
      double minStripPt_;
      int verbosity_;
    };
  }
}

#endif
