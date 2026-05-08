package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Message;

class PingMessage extends Message {
	enum Type {
		PING,
		ACK,
		PING_REQ,
		PING_REQ_ACK
	}

	private final Type type;
	private final String target; // Only used for indirect pings

	public PingMessage(Type type) {
		this(type, null);
	}

	public PingMessage(Type type, String target) {
		this.type = type;
		this.target = target;
	}

	public Type getType() {
		return type;
	}

	public String getTarget() {
		return target;
	}
}