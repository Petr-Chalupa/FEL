package cz.cvut.fel.agents.pdv.student;

public class MsgVoteReq extends MsgRAFT {
	private final int lastLogIndex;
	private final int lastLogTerm;

	public MsgVoteReq(int term, int lastLogIndex, int lastLogTerm) {
		super(term);
		this.lastLogIndex = lastLogIndex;
		this.lastLogTerm = lastLogTerm;
	}

	public int getLastLogIndex() {
		return lastLogIndex;
	}

	public int getLastLogTerm() {
		return lastLogTerm;
	}
}
