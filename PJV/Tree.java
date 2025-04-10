package cz.cvut.fel.pjv;

import java.util.ArrayList;
import java.util.Arrays;

import cz.cvut.fel.pjv.Node;

public class Tree {
    private ArrayList<Node> nodes = new ArrayList<>();

    public void setTree(int[] values) {
        setSubtree(values);
    }

    private Node setSubtree(int[] values) {
        if (values.length == 0) {
            return null;
        }

        int rootIndex = (values.length / 2);

        Node root = new Node(values[rootIndex]);
        Node left = setSubtree(Arrays.copyOfRange(values, 0, rootIndex));
        Node rigth = setSubtree(Arrays.copyOfRange(values, rootIndex + 1, values.length));

        root.setLeft(left);
        root.setRight(rigth);

        nodes.add(root);

        return root;
    }

    public Node getRoot() {
        return this.nodes.size() > 0 ? this.nodes.getLast() : null;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder("");
        sb.append(subtreeToString(getRoot(), 0));
        return sb.toString();
    }

    private String subtreeToString(Node root, int depth) {
        if (root == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder(" ".repeat(depth) + "- " + root.getValue() + "\n");
        sb.append(subtreeToString(root.getLeft(), depth + 1));
        sb.append(subtreeToString(root.getRight(), depth + 1));
        return sb.toString();
    }
}
