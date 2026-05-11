package cz.cvut.fel.agents.pdv.exclusion;

import cz.cvut.fel.agents.pdv.clocked.ClockedMessage;

public class ExclusionMsg extends ClockedMessage {
	public enum Type {
		REQUEST,
		OK
	}

	private final String criticalSectionName;
	private final Type type;

	public ExclusionMsg(String criticalSectionName, Type type) {
		this.criticalSectionName = criticalSectionName;
		this.type = type;
	}

	public String getCriticalSectionName() {
		return criticalSectionName;
	}

	public Type getType() {
		return type;
	}
}
