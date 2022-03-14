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

      const std::vector<std::string>&
      getInputLayerNames() const;

      const std::vector<std::string>&
      getJetInputs() const;

      const std::vector<std::string>&
      getPFCandInputs() const;

      const std::vector<std::string>&
      getPointInputs() const;

      const std::vector<std::string>&
      getMaskInputs() const;

      const unsigned&
      getMaxNumPFCands() const;

      const std::vector<int>&
      getJetConstituent_order() const;

      const std::string&
      getOutputLayerName() const;

      bool
      isGNN() const;

      static void
      fillDescriptions(edm::ParameterSetDescription& desc);

     private:
      std::atomic<tensorflow::GraphDef*> graph_;
      std::vector<std::string> inputLayerNames_;
      std::vector<std::string> jetInputs_;
      std::vector<std::string> pfCandInputs_;
      std::vector<std::string> pointInputs_;
      std::vector<std::string> maskInputs_;
      unsigned maxNumPFCands_;
      std::vector<int> jetConstituent_order_;
      std::string outputLayerName_;
      bool isGNN_;
    };
  }
}

#endif
