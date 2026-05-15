package cz.cvut.fel.agents.pdv.student;

import java.util.HashMap;
import java.util.Map;

public class KeyValueDB {
	private final Map<String, String> data;

	public KeyValueDB() {
		this.data = new HashMap<>();
	}

	public String get(String key) {
		return data.get(key);
	}

	public void put(String key, String value) {
		data.put(key, value);
	}

	public void append(String key, String value) {
		data.merge(key, value, (oldVal, newVal) -> oldVal + newVal);
	}

	public Map<String, String> toMap() {
		return new HashMap<>(data);
	}
}
