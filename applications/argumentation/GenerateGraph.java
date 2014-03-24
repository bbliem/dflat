import java.io.IOException;
import java.util.Random;

/**
 *
 * Graph Generator - Obtain directed graphs
 *
 * @author Wolfgang Dovorak
 * @author Johannes Wallner
 * @author Guenther Charwat
 */
public class GenerateGraph {

		public static final String GENERATOR_VERSION = "2.1.0";

		private enum GraphType {
			G4grid, G8grid, G4graph, G8graph, cliques, random
		}

		private enum EnhancedGraph {
			none, scm
		}

		// asp: standard graph with node, edge
		// aspartix: with arg, att
		private enum OutputFormat {
			asp, aspartix, comparg
		}

		/* Default values/Parameters */
		private static GraphType graphType = GraphType.G4grid;
		private static Integer size = null;
		private static Integer theoreticalTW = null;
		private static Double probability = null;
		private static OutputFormat outputFormat = OutputFormat.asp;
		private static EnhancedGraph enhancedGraph = EnhancedGraph.none;
		private static Double scmLabelProbability = null;

		private static Random random = new Random();
		/**
		 * Generates a graph that is the disjoint union of cliques
		 * @param numberCliques #Cliques
		 * @param treeWidth #tree-width, i.e. size of the cliques-1
		 * @return graph of tree-width  treeWidth and size  numberCliques*(treeWidth+1)
		 */
		private static Graph<String> generateGraphCliques(int numberCliques,int treeWidth){
			Graph<String> graph=new Graph<String>();
			for(int i=0;i<numberCliques;i++){
				for(int j=0;j<treeWidth+1;j++){
					graph.addNode("c"+i+"a"+j);
					for(int k=0;k<j;k++){
						graph.addEdge("c"+i+"a"+k,"c"+i+"a"+j);
						graph.addEdge("c"+i+"a"+j,"c"+i+"a"+k);
					}
				}
			}
			return graph;
		}

		/**
		 * Generates a graph that is an induced subgraph of a complete grid (4-neighbourhood)
		 * (guarantees an upper bound for the tree-width)
		 *
		 * @param n #nodes in the graph
		 * @param treeWidth maximal tree-width of the graph
		 * @param p probability that a possible (directed) edge is in the graph
		 * @return
		 */
		private static Graph<Integer> generateGraphG4(int n,int treeWidth, double p){
			Graph<Integer> graph=new Graph<Integer>();
			int j;
			for(int i=0;i<n;i++){
				graph.addNode(i);

				j=i-treeWidth;
				if (j>=0){
					if( random.nextDouble()<=p ) graph.addEdge(i, j);
					if( random.nextDouble()<=p ) graph.addEdge(j, i);
				}

				j=i-1;
				if (i % treeWidth != 0){
					if( random.nextDouble()<=p ) graph.addEdge(i, j);
					if( random.nextDouble()<=p ) graph.addEdge(j, i);
				}


			}
			return graph;
		}

		/**
		 * Generates a graph that is an induced subgraph of a complete grid (8-neighbourhood)
		 * (guarantees an upper bound for the tree-width)
		 *
		 * @param n #nodes in the graph
		 * @param treeWidth maximal tree-width of the graph
		 * @param p probability that a possible (directed) edge is in the graph
		 * @return
		 */
		private static Graph<Integer> generateGraphG8(int n,int treeWidth, double p){
			Graph<Integer> graph=new Graph<Integer>();
			int k= treeWidth-1;

			int j;
			for(int i=0;i<n;i++){
				graph.addNode(i);

				j=i-k; // lower edge
				if (j>=0){
					if( random.nextDouble()<=p ) graph.addEdge(i, j);
					if( random.nextDouble()<=p ) graph.addEdge(j, i);
				}

				j=i-1; //left edge
				if (i % k != 0){
					if( random.nextDouble()<=p ) graph.addEdge(i, j);
					if( random.nextDouble()<=p ) graph.addEdge(j, i);

					j=i-k-1; //left lower edge
					if(j>=0){
						if( random.nextDouble()<=p ) graph.addEdge(i, j);
						if( random.nextDouble()<=p ) graph.addEdge(j, i);
					}
				}
				j=i-k+1; //right lower edge
				if (j % k != 0 && j>=0){
					if( random.nextDouble()<=p ) graph.addEdge(i, j);
					if( random.nextDouble()<=p ) graph.addEdge(j, i);
				}


			}
			return graph;
		}


		/**
		 * Generates a grid with random orientation (8-neighbourhood)
		 * @param n #nodes in the graph
		 * @param treeWidth tree-width of the graph
		 * @param p probability that an arbitrary edge is symmetric
		 * @return
		 */
		private static Graph<Integer> generateGridG8(int n,int treeWidth, double p){
			Graph<Integer> graph=new Graph<Integer>();
			int k= treeWidth-1;

			int j;
			for(int i=0;i<n;i++){
				graph.addNode(i);

				j=i-k; // lower edge
				if (j>=0){
					if( random.nextDouble()<p) {
						graph.addEdge(i, j);
						graph.addEdge(j, i);
					} else{
						if( random.nextDouble()<1/2) {
							graph.addEdge(i, j);
						} else{
							graph.addEdge(j, i);
						}
					}
				}

				j=i-1; //left edge
				if (i % k != 0){
					if( random.nextDouble()<p) {
						graph.addEdge(i, j);
						graph.addEdge(j, i);
					} else{
						if( random.nextDouble()<1/2) {
							graph.addEdge(i, j);
						} else{
							graph.addEdge(j, i);
						}
					}

					j=i-k-1; //left lower edge
					if (j>=0){
						if( random.nextDouble()<p) {
							graph.addEdge(i, j);
							graph.addEdge(j, i);
						} else{
							if( random.nextDouble()<1/2) {
								graph.addEdge(i, j);
							} else{
								graph.addEdge(j, i);
							}
						}
					}
				}
				j=i-k+1; //right lower edge
				if (j % k != 0 && j>=0){
					if( random.nextDouble()<p) {
						graph.addEdge(i, j);
						graph.addEdge(j, i);
					} else{
						if( random.nextDouble()<1/2) {
							graph.addEdge(i, j);
						} else{
							graph.addEdge(j, i);
						}
					}
				}


			}
			return graph;
		}

		/**
		 * Generates a grid with random orientation (4-neighbourhood)
		 * @param n #nodes in the graph
		 * @param treeWidth tree-width of the graph
		 * @param p probability that an arbitrary edge is symmetric
		 * @return
		 */
		private static Graph<Integer> generateGridG4(int n,int treeWidth, double p){
			Graph<Integer> graph=new Graph<Integer>();
			int j;
			for(int i=0;i<n;i++){
				graph.addNode(i);

				j=i-treeWidth;
				if (j>=0){
					if( random.nextDouble()<p) {
						graph.addEdge(i, j);
						graph.addEdge(j, i);
					} else{
						if( random.nextDouble()<1/2) {
							graph.addEdge(i, j);
						} else{
							graph.addEdge(j, i);
						}
					}
				}

				j=i-1;
				if (i % treeWidth != 0){
					if( random.nextDouble()<p) {
						graph.addEdge(i, j);
						graph.addEdge(j, i);
					} else{
						if( random.nextDouble()<1/2) {
							graph.addEdge(i, j);
						} else{
							graph.addEdge(j, i);
						}
					}
				}


			}
			return graph;
		}
		/**
		 * Generate arbitrary Random DiGraph
		 * @param n #vertices
		 * @param p Propability that an arc (i,j) is in the graph,
		 * 			i.e. the estimated edge density)
		 * @return
		 */
		private static Graph<Integer> generateGraph(int n, double p){
			Graph<Integer> graph=new Graph<Integer>();

			for(int i=0;i<n;i++){
				graph.addNode(i);
			}

			for(int i=0;i<n;i++){
				for(int j=0;j<n;j++){
					if(i!=j & random.nextDouble()<p){
						graph.addEdge(i, j);
					}
				}
			}

			return graph;
		}

		private static String compargGraph(Graph<?> graph){
			StringBuffer stringBuffer= new StringBuffer();

			for(Graph.Node<?> node : graph.getNodes()){
				stringBuffer.append(node.getObject().toString());
				for(Graph.Node<?> child: node.getChildren()){
					stringBuffer.append(" "+child.getObject().toString());
				}
				stringBuffer.append(",\n");
			}

			return stringBuffer.toString();
		}

		private static String aspGraphHeader() {
			StringBuffer stringBuffer= new StringBuffer();
			stringBuffer.append("% Gen. Vers.    : " + GENERATOR_VERSION   + "\n");
			stringBuffer.append("% Graph Type    : " + graphType.name()    + "\n");
			stringBuffer.append("% Size          : " + size 			   + "\n");
			stringBuffer.append("% Th. TW        : " + theoreticalTW       + "\n");
			stringBuffer.append("% Prob.         : " + probability		   + "\n");
			stringBuffer.append("% Output        : " + outputFormat.name() + "\n");
			stringBuffer.append("% Enhanced Graph: " + enhancedGraph.name() + "\n");
			return stringBuffer.toString();
		}

		private static String aspSCMGraphHeader() {
			return "% Label Prob.   : " + scmLabelProbability + "\n";
		}

		private static String aspGraph(Graph<?> graph){
			StringBuffer stringBuffer= new StringBuffer();

			for(Graph.Node<?> node : graph.getNodes()){
				stringBuffer.append("vertex("+node.getObject().toString()+").\n");
				for(Graph.Node<?> child: node.getChildren()){
					stringBuffer.append("edge("+node.getObject().toString()+","+child.getObject().toString()+").\n");
				}
			}

			return stringBuffer.toString();
		}

		private static String aspartixGraph(Graph<?> graph) {
			StringBuffer stringBuffer= new StringBuffer();

			for(Graph.Node<?> node : graph.getNodes()){
				stringBuffer.append("arg("+node.getObject().toString()+").\n");
				for(Graph.Node<?> child: node.getChildren()){
					stringBuffer.append("att("+node.getObject().toString()+","+child.getObject().toString()+").\n");
				}
			}

			return stringBuffer.toString();
		}

		private static String scmGraph(Graph<?> graph, double pl) {
			StringBuffer stringBuffer= new StringBuffer();
			String sign = null;
			for(Graph.Node<?> node : graph.getNodes()){
				if (random.nextDouble() <= pl) {
					sign = (random.nextBoolean() ? "-1" : "1");
					stringBuffer.append("observedV("+node.getObject().toString()+","+sign+").\n");
				}
				for(Graph.Node<?> child: node.getChildren()){
					if (random.nextDouble() <= pl) {
						sign = (random.nextBoolean() ? "-1" : "1");
						stringBuffer.append("observedE("+node.getObject().toString()+","+child.getObject().toString()+","+sign+").\n");
					}
				}
			}

			return stringBuffer.toString();
		}


		public static String pretty(Integer zahl){
			String hilf="000000"+zahl.toString();
			return hilf.substring(hilf.length()-7);
		}


		public static void main(String[] args) throws IOException {


			if (args.length % 2 != 0) {
				printUsage();
				System.exit(1);
			}


			for (int i = 0; i < args.length; i = i+2) {
				if (args[i].equals("-g")) {
					GraphType gttmp = null;
					String t = args[i+1];
					for (GraphType gt : GraphType.values()) {
						if (gt.name().equals(t))
							gttmp = gt;
					}
					if (gttmp == null) {
						System.err.println("Invalid graph type");
						System.exit(1);
					}
					graphType = gttmp;
				}
				else if (args[i].equals("-n")) {
					if (size != null) printUsage();
					try {
						size = Integer.valueOf(args[i+1]);
					} catch (NumberFormatException e) {
						printUsage();
					}
					if (size <= 0 ) {
						System.err.println("Size must be larger than 0");
						System.exit(1);
					}
				}
				else if (args[i].equals("-t")) {
					if (theoreticalTW != null) printUsage();
					try {
						theoreticalTW = Integer.valueOf(args[i+1]);
					} catch (NumberFormatException e) {
						printUsage();
					}
					if (theoreticalTW <= 0 ) {
						System.err.println("Theoretical treewidth must be larger than 0");
						System.exit(1);
					}
				}
				else if (args[i].equals("-p")) {
					try {
						probability = Double.valueOf(args[i+1]);
					} catch (NumberFormatException e) {
						printUsage();
					}
					if (probability < 0 || probability > 1) {
						System.err.println("Probability must be between 0 and 1");
						System.exit(1);
					}
				}
				else if (args[i].equals("-o")) {
					String o = args[i+1];
					OutputFormat oftmp = null;
					for (OutputFormat of : OutputFormat.values()) {
						if (of.name().equals(o))
							oftmp = of;
					}
					if (oftmp == null) {
						System.err.println("Invalid output format");
						System.exit(1);
					}
					outputFormat = oftmp;
				}
				else if (args[i].equals("--scm")) {
					enhancedGraph = EnhancedGraph.scm;
					try {
						scmLabelProbability = Double.valueOf(args[i+1]);
					} catch (NumberFormatException e) {
						printUsage();
					}
					if (scmLabelProbability < 0 || scmLabelProbability > 1) {
						System.err.println("Label probability must be between 0 and 1");
						System.exit(1);
					}
				}
				else {
					printUsage();
				}
			}

			if (graphType == null || outputFormat == null)
				printUsage();

			Graph<?> generatedGraph = null;
			switch (graphType) {
			case G4graph:
				assureSizeSet();
				assureTWSet();
				assureProbabilitySet();
				generatedGraph = generateGraphG4(size, theoreticalTW, probability);
				break;
			case G4grid:
				assureSizeSet();
				assureTWSet();
				assureProbabilitySet();
				generatedGraph = generateGridG4(size, theoreticalTW, probability);
				break;
			case G8graph:
				assureSizeSet();
				assureTWSet();
				assureProbabilitySet();
				generatedGraph = generateGraphG8(size, theoreticalTW, probability);
				break;
			case G8grid:
				assureSizeSet();
				assureTWSet();
				assureProbabilitySet();
				generatedGraph = generateGridG8(size, theoreticalTW, probability);
				break;
			case random:
				assureSizeSet();
				assureProbabilitySet();
				generatedGraph = generateGraph(size, probability);
				break;
			case cliques:
				assureSizeSet();
				assureTWSet();
				generatedGraph = generateGraphCliques(size, theoreticalTW);
				break;
			default:
				printUsage();
			}

			String header = "";
			String graphString = "";
			switch (outputFormat) {
			case asp:
				header = aspGraphHeader();
				graphString = aspGraph(generatedGraph);
				break;
			case aspartix:
				header = aspGraphHeader();
				graphString = aspartixGraph(generatedGraph);
				break;
			case comparg:
				graphString = compargGraph(generatedGraph);
				break;
			}


			switch (enhancedGraph) {
			case none:
				break;
			case scm:
				switch (outputFormat) {
				case asp:
				case aspartix:
					assureSCMLabelProbabilitySet();
					header += aspSCMGraphHeader();
					graphString += "\n" + scmGraph(generatedGraph, scmLabelProbability);
					break;
				default:
					System.err.println("Output format not compatible with SCM\n");
					System.exit(1);
				}
			}

			System.out.println(header);
			System.out.println(graphString);
		}

		private static void assureSizeSet() {
			if (size == null) {
				System.err.println("Size is not set\n");
				System.exit(1);
			}
		}

		private static void assureProbabilitySet() {
			if (probability == null) {
				System.err.println("Probability is not set\n");
				System.exit(1);
			}
		}

		private static void assureTWSet() {
			if (theoreticalTW == null) {
				System.err.println("Theoretical treewidth is not set\n");
				System.exit(1);
			}
		}

		private static void assureSCMLabelProbabilitySet() {
			if (scmLabelProbability == null) {
				System.err.println("Label probability is not set\n");
				System.exit(1);
			}
		}

		private static void printUsage() {
			String usage = "java GenerateGraph [Options]\n";
			usage += "Prints a graph to stdout\n";
			usage += "[-g <graphType>]     One of G4grid, G8grid, G4graph, G8graph, cliques, random\n";
			usage += "                      G4grid  ... 4-connected full grid, prob. influences symmetry of edges (Default)\n";
			usage += "                      G8grid  ... 8-connected full grid, prob. influences symmetry of edges\n";
			usage += "                      G4graph ... 4-connected grid-like graph, prob. influences existence of edges\n";
			usage += "                      G8graph ... 8-connected grid-like graph, prob. influences existence of edges\n";
			usage += "                      cliques ... Creates <size> cliques with <treewidth>+1 nodes each, prob. ignored\n";
			usage += "                      random  ... Random graph, edges exist based on prob., treewidth ignored\n";
			usage += "[-n <size>]          Number of nodes (Grid, Graph, Random) or number of Cliques (clique)\n";
			usage += "[-t <treewidth>]     Treewidth (Grid, Cliques), Max. Treewidth (Graph) or ignored (Random)\n";
			usage += "[-p <probability>]   Symmetry (Grid), Edge (Graph, Random) probability or ignored (clique)\n";
			usage += "[-o <outputFormat>]  One of asp (Default), aspartix, comparg\n";
			usage += "[--scm <labelprob>]  Sign Consistency Model graph with <labelprob> probability that \n";
			usage += "                     a node/edge gets assigned an observed label (-1,1) randomly \n";

			System.err.println(usage);
			System.exit(1);
		}
}
