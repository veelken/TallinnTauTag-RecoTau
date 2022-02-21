#ifndef TallinnTauTag_RecoTau_TallinnTauPiZeroBuilder_h
#define TallinnTauTag_RecoTau_TallinnTauPiZeroBuilder_h

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h" // edm::ParameterSetDescription 

#include "DataFormats/TauReco/interface/RecoTauPiZero.h"           // reco::RecoTauPiZero
#include "DataFormats/TauReco/interface/RecoTauPiZeroFwd.h         // reco::RecoTauPiZeroCollection

#include "TFormula.h"

namespace reco {
  namespace tau {

    class TallinnTauPiZeroBuilder
    {
     public:
      explicit TallinnTauPiZeroBuilder(const edm::ParameterSet& cfg);
      ~TallinnTauPiZeroBuilder();

      reco::RecoTauPiZeroCollection 
      operator()(const std::vector<edm::Ptr<reco::PFCandidate>>& pfCands, 
                 const reco::Vertex::Point& primaryVertexPos);

      static void 
      fillDescriptions(edm::ParameterSetDescription& desc);

     private:
      std::vector<int> inputParticleIds_;
      std::unique_ptr<const TFormula> etaAssociationDistance_;
      std::unique_ptr<const TFormula> phiAssociationDistance_;
      int verbosity_;
    };
  }
}

#endif
