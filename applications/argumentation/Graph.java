

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
/**
 * A directed graph
 *
 * @author Wolfgang Dvorak
 *
 * @param <NTyp> type of the content saved in graph nodes
 */
public class Graph<NTyp> implements Iterable<NTyp>{
	/**
	 * Node in a directed graph
	 *
	 * @author Wolfgang Dvorak
	 *
	 * @param <NTyp> type of the content saved in graph nodes
	 */
	public static class Node<NTyp>{
		private NTyp object;
		private Set<Node<NTyp>> parents;
		private Set<Node<NTyp>> childs;

		public Node(NTyp object){
			this.object=object;
			this.parents=new HashSet<Node<NTyp>>();
			this.childs=new HashSet<Node<NTyp>>();
		}

		public NTyp getObject(){
			return this.object;
		}

		public Set<Node<NTyp>> getChildren(){
			return this.childs;
		}

		public Set<Node<NTyp>> getParents(){
			return this.parents;
		}

		public boolean isLinked(){
			return !(this.parents.isEmpty() && this.childs.isEmpty());
		}

		public boolean addParent(Node<NTyp> parent){
			if(!this.parents.contains(parent)){
				this.parents.add(parent);
				return true;
			}else{
				return false;
			}
		}
		public boolean addChild(Node<NTyp> child){
			if(!this.childs.contains(child)){
				this.childs.add(child);
				return true;
			}else{
				return false;
			}
		}

		public boolean removeParent(Node<NTyp> parent){
			return this.parents.remove(parent);
		}

		public boolean removeChild(Node<NTyp> child){
			return this.childs.remove(child);
		}

		public Iterator<Node<NTyp>> parentIterator(){
			return this.parents.iterator();
		}

		public Iterator<Node<NTyp>> childIterator(){
			return this.childs.iterator();
		}

		public String toString() {
			return object.toString();
		}
	}
	private Map<NTyp,Node<NTyp>> nodes;

	public Graph() {
		this.nodes=new HashMap<NTyp,Node<NTyp>>();
	}

	public Iterator<NTyp> iterator() {
		return nodes.keySet().iterator();
	}

	public boolean addNode(NTyp node){
		if (nodes.containsKey(node)){
			return false;
		}
		nodes.put(node, new Node<NTyp>(node));
		return true;
	}

	public boolean removeNode(NTyp node){
		if (nodes.containsKey(node)){
			Node<NTyp> mnode = nodes.get(node);
			nodes.remove(node);
			// remove edges
			for(Node<NTyp> mnode2 : nodes.values()){
				mnode2.removeChild(mnode);
				mnode2.removeChild(mnode);
			}
			return true;
		}
		return false;
	}

	public boolean addEdge(NTyp node1,NTyp node2){
		if (nodes.containsKey(node1) & nodes.containsKey(node2)){
			nodes.get(node1).addChild(nodes.get(node2));
			nodes.get(node2).addParent(nodes.get(node1));
			return true;
		} else {
			return false;
		}
	}

	public boolean removeEdge(NTyp node1,NTyp node2){
		if (nodes.containsKey(node1) & nodes.containsKey(node2)){
			return nodes.get(node1).removeChild(nodes.get(node2)) &
			nodes.get(node2).removeParent(nodes.get(node1));
		} else {
			return false;
		}
	}

	public Node<NTyp> get(NTyp node) {
		return nodes.get(node);
	}

	/**
	 *
	 * @return number of nodes
	 */
	public int size() {
		return nodes.size();
	}

	public Collection<Node<NTyp>> getNodes() {
		return nodes.values();
	}
}
