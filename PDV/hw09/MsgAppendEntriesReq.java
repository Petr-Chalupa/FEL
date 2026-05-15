package cz.cvut.fel.agents.pdv.student;


import java.util.List;

public class MsgAppendEntriesReq extends MsgRAFT {
	private final int prevLogIndex;
	private final int prevLogTerm;
	private final int leaderCommit;
	private final List<LogEntry> entries;

	public MsgAppendEntriesReq(int term, int prevLogIndex, int prevLogTerm, int leaderCommit, List<LogEntry> entries) {
		super(term);
		this.prevLogIndex = prevLogIndex;
		this.prevLogTerm = prevLogTerm;
		this.leaderCommit = leaderCommit;
		this.entries = entries;
	}

	public int getPrevLogIndex() {
		return prevLogIndex;
	}

	public int getPrevLogTerm() {
		return prevLogTerm;
	}

	public int getLeaderCommit() {
		return leaderCommit;
	}

	public List<LogEntry> getEntries() {
		return entries;
	}

	public boolean isHeartbeat() {
		return entries.isEmpty();
	}
}
