package cz.cvut.fel.agents.pdv.student;


public class MsgAppendEntriesRes extends MsgRAFT {
	private final boolean success;
	private final int lastLogIndex; // If success: the highest log index matching the leader. If failure: follower's log size to optimize backtracking (skipping non-existent indexes).

	public MsgAppendEntriesRes(int term, boolean success, int lastLogIndex) {
		super(term);
		this.success = success;
		this.lastLogIndex = lastLogIndex;
	}

	public boolean isSuccess() {
		return success;
	}

	public int getLastLogIndex() {
		return lastLogIndex;
	}
}
