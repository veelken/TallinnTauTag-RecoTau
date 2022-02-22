#ifndef TallinnTauTag_RecoTau_TallinnTauCache_h
#define TallinnTauTag_RecoTau_TallinnTauCache_h

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h" // edm::ParameterSetDescription
#include "FWCore/ParameterSet/interface/ParameterSet.h"            // edm::VParameterSet

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"          // tensorflow::GraphDef, tensorflow::Session

namespace reco 
{
  namespace tau 
  {
    class TallinnTauCache 
    {
     public:
      TallinnTauCache(const edm::VParameterSet& cfg_graphs);
      ~TallinnTauCache();

      // A Session allows concurrent calls to Run(), though a Session must
      // be created / extended by a single thread.
      tensorflow::Session& 
      getSession(const std::string& name = "") const;

      const tensorflow::GraphDef& 
      getGraph(const std::string& name = "") const;

      static void 
      fillDescriptions(edm::ParameterSetDescription& desc);

     private:
      std::map<std::string, std::shared_ptr<tensorflow::GraphDef>> graphs_;
      std::map<std::string, tensorflow::Session*> sessions_;
    };
  }
}

#endif
