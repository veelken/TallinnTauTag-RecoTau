#ifndef TallinnTauTag_RecoTau_TFGraphCache_h
#define TallinnTauTag_RecoTau_TFGraphCache_h

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h" // edm::ParameterSetDescription
#include "FWCore/ParameterSet/interface/ParameterSet.h"            // edm::VParameterSet

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"          // tensorflow::GraphDef, tensorflow::Session

namespace reco 
{
  namespace tau 
  {
    class TFGraphCache 
    {
     public:
      TFGraphCache(const edm::ParameterSet& cfg);
      ~TFGraphCache();

      const tensorflow::GraphDef& 
      getGraph() const;

      const std::string&
      getInputLayerName() const;

      const std::string&
      getOutputLayerName() const;

      static void 
      fillDescriptions(edm::ParameterSetDescription& desc);

     private:
      std::atomic<tensorflow::GraphDef*> graph_;
      std::string inputLayerName_;
      std::string outputLayerName_;
    };
  }
}

#endif
