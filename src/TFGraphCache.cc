#include "TallinnTauTag/RecoTau/interface/TFGraphCache.h"

#include "FWCore/Utilities/interface/Exception.h" // edm::Exception

using namespace reco::tau;

namespace
{
  std::string
  format_vstring(const std::vector<std::string>& values)
  {
    std::ostringstream output;
    output << "[ ";
    for ( size_t idxValue = 0; idxValue < values.size(); ++idxValue )
    {
      if ( idxValue != 0 )
      {
        output << ", ";
      }
      output << values[idxValue];
    }
    output << " ]";
    return output.str();
  }
}

TFGraphCache::TFGraphCache(const edm::ParameterSet& cfg)
  : inputLayerNames_(cfg.getParameter<std::vector<std::string>>("inputLayerNames"))
  , jetInputs_(cfg.getParameter<std::vector<std::string>>("jetInputs"))
  , pfCandInputs_(cfg.getParameter<std::vector<std::string>>("pfCandInputs"))
  , pointInputs_(cfg.getParameter<std::vector<std::string>>("pointInputs"))
  , maskInputs_(cfg.getParameter<std::vector<std::string>>("maskInputs"))
  , maxNumPFCands_(cfg.getParameter<unsigned>("maxNumPFCands"))
  , jetConstituent_order_(cfg.getParameter<std::vector<int>>("jetConstituent_order"))
  , outputLayerName_(cfg.getParameter<std::string>("outputLayerName"))
{
  std::string inputFileName = cfg.getParameter<std::string>("inputFile");
  std::string inputFileName_full = edm::FileInPath(inputFileName).fullPath();
  isGNN_ = cfg.getParameter<bool>("gnn");
  std::string graphName = cfg.getParameter<std::string>("graphName");
  std::cout << "<TFGraphCache::TFGraphCache>: loading graph = '" << graphName << "' from input file = '" << inputFileName_full << "'." << std::endl;
  graph_ = tensorflow::loadGraphDef(inputFileName_full);
  assert(graph_);

  // CV: read names of DNN input and output layers directly from the TensorFlow graph
  //inputLayerNames_ = { (*graph_).node(0).name() }; // CV: this only works for DNN, not for GNN !!
  outputLayerName_ = (*graph_).node((*graph_).node_size() - 1).name();
  std::cout << " inputLayerNames = " << format_vstring(inputLayerNames_) << std::endl;
  std::cout << " outputLayerName = " << outputLayerName_ << std::endl;

  // set tensorflow verbosity to warning level
  tensorflow::setLogging("2");
}

TFGraphCache::~TFGraphCache()
{
  delete graph_;
  graph_ = nullptr;
}

const tensorflow::GraphDef&
TFGraphCache::getGraph() const
{
  return *graph_;
}

const std::vector<std::string>&
TFGraphCache::getInputLayerNames() const
{
  return inputLayerNames_;
}

const std::vector<std::string>&
TFGraphCache::getJetInputs() const
{
  return jetInputs_;
}

const std::vector<std::string>&
TFGraphCache::getPFCandInputs() const
{
  return pfCandInputs_;
}

const std::vector<std::string>&
TFGraphCache::getPointInputs() const
{
  return pointInputs_;
}

const std::vector<std::string>&
TFGraphCache::getMaskInputs() const
{
  return maskInputs_;
}

const unsigned&
TFGraphCache::getMaxNumPFCands() const
{
  return maxNumPFCands_; 
}

const std::vector<int>&
TFGraphCache::getJetConstituent_order() const
{
  return jetConstituent_order_;
}

bool
TFGraphCache::isGNN() const
{
  return isGNN_;
}

const std::string&
TFGraphCache::getOutputLayerName() const
{
  return outputLayerName_;
}

void
TFGraphCache::fillDescriptions(edm::ParameterSetDescription& desc)
{
  desc.add<std::string>("inputFile", "");
  desc.add<std::string>("graphName", "");
  desc.add<std::vector<std::string>>("inputLayerNames", { "input" });
  desc.add<std::string>("outputLayerName", "output");
  desc.add<std::vector<std::string>>("jetInputs", {});
  desc.add<std::vector<std::string>>("pfCandInputs", {});
  desc.add<std::vector<std::string>>("pointInputs", { "dEta_jet", "dPhi_jet" });
  desc.add<std::vector<std::string>>("maskInputs", { "log" });
  desc.add<bool>("gnn", false);
  desc.add<unsigned>("maxNumPFCands", 20);
  desc.add<std::vector<int>>("jetConstituent_order", { 1, 2, 4, 3, 5 }); // h, e, gamma, mu, h0
}
