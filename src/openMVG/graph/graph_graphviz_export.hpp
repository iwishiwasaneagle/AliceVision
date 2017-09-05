// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef OPENMVG_GRAPH_EXPORT_H_
#define OPENMVG_GRAPH_EXPORT_H_

#include <iostream>
#include <fstream>
#include <cstdlib>

namespace openMVG {
namespace graph {

// Export an Image connection graph
//   to graphviz file format.
// Example :
// graph 1 {
//   node [shape=circle]
//   n2
//   n1
//   n0
//   n1 --  n0
//   n2 --  n0
//   n2 --  n1
// }
template <typename GraphT>
bool exportToGraphvizFormat_Nodal(
  const GraphT & g,
  std::ostream & os)
{
  os << "graph 1 {" << std::endl;
  os << "node [shape=circle]" << std::endl;
  //Export node label
  for(typename GraphT::NodeIt n(g); n!= lemon::INVALID; ++n)
  {
    os << "  n" << g.id(n) << std::endl;
  }

  //-- Export arc (as the graph is bi-directional, export arc only one time)

  std::map< std::pair<IndexT, IndexT>, IndexT > map_arcs;
  for(typename GraphT::ArcIt e(g); e!=lemon::INVALID; ++e) {
    if( map_arcs.end() == map_arcs.find(std::make_pair(IndexT(g.id(g.source(e))), IndexT(g.id(g.target(e)))))
      &&
      map_arcs.end() == map_arcs.find(std::make_pair(IndexT(g.id(g.target(e))), IndexT(g.id(g.source(e))))))
    {
      map_arcs[std::make_pair(IndexT(g.id(g.source(e))),IndexT(g.id(g.target(e)))) ] = 1;
    }
  }
  //os << "edge [style=bold]" << endl;
  for (std::map< std::pair<IndexT, IndexT>, IndexT >::const_iterator iter = map_arcs.begin();
    iter != map_arcs.end();
    ++iter)
  {
    os << "  n" << iter->first.first << " -- " << " n" << iter->first.second << std::endl;
  }

  os << "}" << std::endl;
  return os.good();
}

// Export the Image connection graph
//   to the graphviz file format.
// Add the image name and preview in the graph by using HTML label.
template <typename GraphT, typename NodeMap, typename EdgeMap>
bool exportToGraphvizFormat_Image(
  const GraphT & g,
  const NodeMap & nodeMap,
  const EdgeMap & edgeMap,
  std::ostream & os, bool bWeightedEdge = false)
{
  os << "graph 1 {" << std::endl;
  os << "node [shape=none]" << std::endl;
  //Export node label
  for(typename GraphT::NodeIt n(g); n!=lemon::INVALID; ++n)
  {
    os << "  n" << g.id(n)
      << "[ label ="
      << "< "<< std::endl
      << "<table>" << std::endl
      << "<tr><td>" << "\"" << nodeMap[n] << "\"" << "</td></tr>" << std::endl
      << "<tr><td><img src=\"" << nodeMap[n] << "\"/></td></tr>" << std::endl
      << "</table>" << std::endl
      << ">, cluster=1];" << std::endl;

    //os << "  n" << g.id(n)
    //  << " [ "
    //  << " image=\"" << nodeMap[n] << "\" cluster=1]; " << endl;
  }

  //Export arc value
  std::map< std::pair<IndexT, IndexT>, IndexT > map_arcs;
  for(typename GraphT::ArcIt e(g); e!=lemon::INVALID; ++e) {
    if( map_arcs.end() == map_arcs.find(std::make_pair(IndexT(g.id(g.source(e))), IndexT(g.id(g.target(e)))))
      &&
      map_arcs.end() == map_arcs.find(std::make_pair(IndexT(g.id(g.target(e))), IndexT(g.id(g.source(e))))))
    {
      map_arcs[std::make_pair(IndexT(g.id(g.source(e))),
        IndexT(g.id(g.target(e)))) ] = edgeMap[e];
    }
  }

  os << "edge [style=bold]" << std::endl;
  for ( std::map< std::pair<IndexT,IndexT>, IndexT>::const_iterator iter = map_arcs.begin();
    iter != map_arcs.end();
    ++iter)
  {
    if (bWeightedEdge)
    {
      os << "  n" << iter->first.first << " -- " << " n" << iter->first.second
        << " [label=\"" << iter->second << "\"]" << std::endl;
    }
    else
    {
      os << "  n" << iter->first.first << " -- " << " n" << iter->first.second << std::endl;
    }
  }
  os << "}" << std::endl;
  return os.good();
}

template <typename GraphT>
void exportToGraphvizData(const std::string& sfile, const GraphT & graph){
  //Prepare Data

  std::ofstream file(sfile.c_str());
  openMVG::graph::exportToGraphvizFormat_Nodal(graph, file);
  file.close();

  //Use Graphviz
  const std::string cmd = "neato -Tsvg -O -Goverlap=scale -Gsplines=false " + sfile;
  int ret = std::system(cmd.c_str());
  (void)ret;
}

} // namespace graph
} // namespace openMVG

#endif // OPENMVG_GRAPH_EXPORT_H_
