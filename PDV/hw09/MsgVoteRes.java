package cz.cvut.fel.agents.pdv.student;

public class MsgVoteRes extends MsgRAFT {
	private final boolean voteGranted;

	MsgVoteRes(int term, boolean voteGranted) {
		super(term);
		this.voteGranted = voteGranted;
	}

	public boolean isVoteGranted() {
		return voteGranted;
	}
}
