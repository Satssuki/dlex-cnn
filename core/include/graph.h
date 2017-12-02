////////////////////////////////////////////////////////////////
// > Copyright (c) 2017 by Contributors. 
// > https://github.com/cjmcv
// > brief  Node graph, support for node operation
//          including forward and backward
// > author Jianming Chen
////////////////////////////////////////////////////////////////

#ifndef DLEX_GRAPH_HPP_
#define DLEX_GRAPH_HPP_

//#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdlib.h>
#include <stack>

#include "util/timer.h"
#include "node.h"

namespace dlex_cnn
{
	template <typename Dtype>
	class Graph
	{
	public:
		explicit Graph();
		virtual ~Graph();

		int getNodeIndex(const std::string &node_name, int &index);
		void addNode(const std::string &node_name,
			const std::vector<std::shared_ptr<Op<Dtype>>> &op,
			const std::vector<std::string> &inNodeNames = std::vector<std::string>() );
		int setIONodeName(const std::vector<std::string> &in_node_names, const std::vector<std::string> &out_node_names);
		// Set input nodes of the graph.
		int setInNode(const std::vector<std::shared_ptr<Tensor<Dtype>>> inputData);	//
		// Set output nodes of the graph.
		int setOutNode(const std::vector<std::shared_ptr<Tensor<Dtype>>> label_data);	//
		int forwardGraph();
		int backwardGraph();
		// Fetch loss that has been saved in one of the output nodes.
		int getLoss(const std::string &node_name, Dtype &loss);
		// Fetch data in the specified node.
		int getNodeData(const std::string &node_name, std::shared_ptr<Tensor<Dtype>> &cpuData);

		int graphShow();
		int writeGraph2Text(FILE *fp);
		int writeGraph2Bin(FILE *fp);
		int writeGraphParam2Bin(FILE *fp);

		int readText2Graph(FILE *fp);
		int readBin2Graph(FILE *fp);
		int readBin2GraphParam(FILE *fp);

		inline void setPhase(int phase) { phase_ = phase; };
		inline const std::vector<std::shared_ptr<Node<Dtype>>> &getGraphNodes() { return nodes_; };

	private:
		int phase_ = tind::Train;
		// The names of input nodes, and the order of vector's elements should matche the input data vector's.
		std::vector<std::string> in_node_names_;
		// The names of output nodes.
		std::vector<std::string> out_node_names_;
		// Bakckup in_node's name and idx
		std::map<std::string, int> in_nodes_map_;
		// Bakckup out_node's name and idx
		std::map<std::string, int> out_nodes_map_;	
		// Bakckup names and idxs of all nodes in graph
		std::map<std::string, int> nodes_index_map_;
		// Nodes list
		std::vector<std::shared_ptr<Node<Dtype>>> nodes_;
		// Temporary stack for DFS in forward and backward
		std::stack<int> nodes_idx_stack_;
	};
}
#endif //DLEX_GRAPH_HPP_
