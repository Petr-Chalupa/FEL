package cz.cvut.fel.agents.pdv.student;

import java.util.ArrayList;
import java.util.List;

public class Log {
	private List<LogEntry> entries;

	public Log() {
		this.entries = new ArrayList<>();
	}

	public LogEntry getEntry(int index) {
		return entries.get(index);
	}

	public LogEntry getLastEntry() {
		return entries.isEmpty() ? null : entries.getLast();
	}

	public int size() {
		return entries.size();
	}

	public void append(LogEntry entry) {
		entries.add(entry);
	}

	public void appendAll(List<LogEntry> entries) {
		this.entries.addAll(entries);
	}

	public void dropFrom(int index) {
		entries = new ArrayList<>(entries.subList(0, index));
	}
}
