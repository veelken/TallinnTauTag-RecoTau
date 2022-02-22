#include "TallinnTauTag/RecoTau/interface/TallinnTauCache.h"

#include "FWCore/Utilities/interface/Exception.h" // edm::Exception

using namespace reco::tau;

TallinnTauCache::TallinnTauCache(const edm::VParameterSet& cfg_graphs)
{
  for ( auto const& cfg_graph : cfg_graphs )
  {
    std::string inputFileName = cfg_graph.getParameter<std::string>("inputFile");    
    std::string graphName = cfg_graph.getParameter<std::string>("graphName");
    if ( graphs_.find(graphName) != graphs_.end() )
      throw cms::Exception("TallinnTauCache") 
        << "Invalid configuration parameters: graph '" << graphName << "' defined more than once !!";
    
    std::string inputFileName_full = edm::FileInPath(inputFileName).fullPath();
    tensorflow::SessionOptions options;
    tensorflow::setThreading(options, 1);
    graphs_[graphName].reset(tensorflow::loadGraphDef(inputFileName_full));
    sessions_[graphName] = tensorflow::createSession(graphs_[graphName].get(), options);
  }
}

TallinnTauCache::~TallinnTauCache()
{
  for ( auto & session : sessions_ )
  {
    tensorflow::closeSession(session.second);
  }
}

tensorflow::Session& 
TallinnTauCache::getSession(const std::string& name) const
{
  auto session = sessions_.find(name);
  if ( session == sessions_.end() )
    throw cms::Exception("TallinnTauCache") 
      << "Invalid session '" << name << "' !!";
  return *session->second;
}

const tensorflow::GraphDef&
TallinnTauCache::getGraph(const std::string& name) const
{
  auto graph = graphs_.find(name);
  if ( graph == graphs_.end() )
    throw cms::Exception("TallinnTauCache") 
      << "Invalid graph '" << name << "' !!";
  return *graph->second;
}

void 
TallinnTauCache::fillDescriptions(edm::ParameterSetDescription& desc)
{
  desc.add<std::string>("inputFile", "");
  desc.add<std::string>("graphName", "");
}
