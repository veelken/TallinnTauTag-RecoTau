#ifndef TallinnTauTag_RecoTau_TallinnTauBuilder_h
#define TallinnTauTag_RecoTau_TallinnTauBuilder_h

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"   // edm::ParameterSetDescription
#include "FWCore/ParameterSet/interface/ParameterSet.h"              // edm::ParameterSet

#include "CommonTools/Utils/interface/StringObjectFunction.h"        // StringObjectFunction

#include "DataFormats/JetReco/interface/PFJet.h"                     // reco::PFJetRef
#include "DataFormats/TauReco/interface/PFTau.h"                     // reco::PFTau
#include "DataFormats/TauReco/interface/PFTauFwd.h"                  // reco::PFTauCollection
#include "DataFormats/VertexReco/interface/Vertex.h"                 // reco::Vertex::Point

#include "TallinnTauTag/RecoTau/interface/TallinnTauPiZeroBuilder.h" // reco::tau::TallinnTauPiZeroBuilder
#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidate.h"      // reco::wrappedPFCandidate, reco::wrappedPFCandidateCollection

#include <vector>  // std::vector
#include <utility> // std::pair

namespace reco 
{
  namespace tau 
  {
    class TallinnTauBuilder
    {
     public:
      explicit TallinnTauBuilder(const edm::ParameterSet& cfg);
      ~TallinnTauBuilder();

      reco::PFTau
      operator()(const reco::PFJetRef& jetRef,
                 const reco::wrappedPFCandidateCollection& signalPFCands,
                 const reco::wrappedPFCandidateCollection& isolationPFCands,
                 const reco::Vertex::Point& primaryVertexPos);

      static void 
      fillDescriptions(edm::ParameterSetDescription& desc);

     private:
      StringObjectFunction<reco::PFTau> signalConeSize_;
      std::vector<int> chargedHadrParticleIds_;
      TallinnTauPiZeroBuilder piZeroBuilder_;
      int verbosity_;
    };
  }
}

#endif
