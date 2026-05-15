package cz.cvut.fel.agents.pdv.student;

import cz.cvut.fel.agents.pdv.dsand.Message;

public abstract class MsgRAFT extends Message {
	private final int term;

	public MsgRAFT(int term) {
		this.term = term;
	}

	public int getTerm() {
		return term;
	}
}
