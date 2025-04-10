package cz.cvut.fel.pjv;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class CircularArrayQueue {
    private String[] items;
    private int head, tail, size;

    /**
     * Creates the queue with capacity set to the value of 5.
     */
    public CircularArrayQueue() {
        this.items = new String[5];
        this.head = 0;
        this.tail = 0;
        this.size = 0;
    }

    /**
     * Creates the queue with given {@code capacity}. The capacity represents
     * maximal number of elements that the
     * queue is able to store.
     * 
     * @param capacity of the queue
     */
    public CircularArrayQueue(int capacity) {
        this.items = new String[capacity];
        this.head = 0;
        this.tail = 0;
        this.size = 0;
    }

    public int size() {
        return this.size;
    }

    public boolean isEmpty() {
        return this.size == 0;
    }

    public boolean isFull() {
        return this.size == this.items.length;
    }

    public boolean enqueue(String obj) {
        if (obj == null || this.isFull())
            return false;

        this.items[this.tail] = obj;
        this.tail = (this.tail + 1) % this.items.length;
        this.size++;

        return true;
    }

    public String dequeue() {
        if (this.isEmpty())
            return null;

        String obj = this.items[this.head];
        this.items[this.head] = null;
        this.head = (this.head + 1) % this.items.length;
        this.size--;

        return obj;
    }

    public Collection<String> getElements() {
        List<String> list = new ArrayList<>();
        for (int i = this.head; i < this.items.length + this.head; i++) {
            String item = this.items[(i % this.items.length)];
            if (item != null) {
                list.add(item);
            }
        }
        return list;
    }

    public void printAllElements() {
        Collection<String> list = this.getElements();
        for (String obj : list) {
            System.out.println(obj);
        }
    }
}
