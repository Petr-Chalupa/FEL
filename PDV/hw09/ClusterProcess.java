package cz.cvut.fel.agents.pdv.student;

import cz.cvut.fel.agents.pdv.dsand.Message;
import cz.cvut.fel.agents.pdv.dsand.Pair;
import cz.cvut.fel.agents.pdv.evaluation.StoreOperationEnums;
import cz.cvut.fel.agents.pdv.raft.RaftProcess;
import cz.cvut.fel.agents.pdv.raft.messages.*;

import java.util.*;
import java.util.function.BiConsumer;

public class ClusterProcess extends RaftProcess<Map<String, String>> {
	public enum State {FOLLOWER, CANDIDATE, LEADER}

	// --- Base config ---
	private final Random random = new Random();
	private final List<String> otherProcessesInCluster;
	private final int networkDelays;
	// --- State ---
	private final KeyValueDB kvdb;
	private State state;
	private final Log log;
	private int currentTerm;
	private String currentLeader;
	private String votedFor;
	private int commitIndex;
	private int lastApplied;
	// --- CANDIDATE state ---
	private int voteCount;
	// --- LEADER state ---
	private Map<String, Integer> nextIndex;
	private Map<String, Integer> matchIndex;
	//	private Map<String, Integer> lastSentMatchIndex;
	// --- Timeouts ---
	private final int MIN_ELECTION_TIMEOUT;
	private final int MAX_ELECTION_TIMEOUT;
	private int electionTimeout;
	private final int heartbeatTimeout;
	private int currentIteration;
	private int lastElection;
	private int lastHeartbeat;

	public ClusterProcess(String id, Queue<Message> inbox, BiConsumer<String, Message> outbox, List<String> otherProcessesInCluster, int networkDelays) {
		super(id, inbox, outbox);
		this.otherProcessesInCluster = otherProcessesInCluster;
		this.networkDelays = networkDelays;

		this.kvdb = new KeyValueDB();
		this.state = State.FOLLOWER;
		this.log = new Log();
		this.currentTerm = 0;
		this.currentLeader = null;
		this.votedFor = null;
		this.commitIndex = -1;
		this.lastApplied = -1;

		this.voteCount = 0;

		this.nextIndex = new HashMap<>();
		this.matchIndex = new HashMap<>();

		this.heartbeatTimeout = (networkDelays * 2) + 5;
		this.MIN_ELECTION_TIMEOUT = heartbeatTimeout * 5;
		this.MAX_ELECTION_TIMEOUT = heartbeatTimeout * 10;
		regenerateElectionTimeout();
		this.currentIteration = 0;
		this.lastElection = 0;
		this.lastHeartbeat = 0;
	}

	@Override
	public Optional<Map<String, String>> getLastSnapshotOfLog() {
		if (log.size() == 0) return Optional.empty();
		return Optional.of(kvdb.toMap()); // Return a snapshot of the whole log
	}

	@Override
	public String getCurrentLeader() {
		return currentLeader;
	}

	public int getQuorum() {
		return ((otherProcessesInCluster.size() + 1) / 2) + 1;
	}

	@Override
	public void act() {
		currentIteration++;

		while (!inbox.isEmpty()) {
			Message message = inbox.poll();
			if (message instanceof MsgAppendEntriesReq msgAppendEntriesReq) {
				handleAppendEntriesRequest(msgAppendEntriesReq);
			} else if (message instanceof MsgAppendEntriesRes msgAppendEntriesRes) {
				handleAppendEntriesResponse(msgAppendEntriesRes);
			} else if (message instanceof MsgVoteReq msgVoteReq) {
				handleVoteRequest(msgVoteReq);
			} else if (message instanceof MsgVoteRes msgVoteRes) {
				handleVoteResponse(msgVoteRes);
			} else if (message instanceof ClientRequest clientRequest) {
				handleClientRequest(clientRequest);
			} else {
				System.out.println("Unknown message: " + message.getClass());
			}
		}

		startElection();
		sendHeartbeats();
		applyLog();
	}

	private void regenerateElectionTimeout() {
		this.electionTimeout = MIN_ELECTION_TIMEOUT + random.nextInt(MAX_ELECTION_TIMEOUT - MIN_ELECTION_TIMEOUT + 1);
		this.lastElection = currentIteration;
	}

	private void changeState(State newState, int newTerm) {
		state = newState;
		currentTerm = Math.max(currentTerm, newTerm);
		regenerateElectionTimeout();
		lastHeartbeat = currentIteration;
		votedFor = null;
		voteCount = 0;

		if (newState == State.FOLLOWER) {
			currentLeader = null;
		} else if (newState == State.CANDIDATE) {
			currentLeader = null;
			votedFor = getId();
			voteCount = 1;
		} else if (newState == State.LEADER) {
			currentLeader = getId();
			nextIndex = new HashMap<>();
			matchIndex = new HashMap<>();
			for (String otherProcess : otherProcessesInCluster) {
				nextIndex.put(otherProcess, log.size());
				matchIndex.put(otherProcess, -1);
			}
		}
	}

	private void updateCommitIndex() {
		for (int i = log.size() - 1; i > commitIndex; i--) {
			int count = 1;

			for (String otherProcess : otherProcessesInCluster) {
				if (matchIndex.getOrDefault(otherProcess, -1) >= i) count++;
			}

			if (count >= getQuorum() && log.getEntry(i).term() == currentTerm) {
				commitIndex = i;
				break;
			}
		}
	}

	private void applyLog() {
		while (lastApplied < commitIndex) {
			lastApplied++;

			LogEntry entry = log.getEntry(lastApplied);
			String opName = entry.operation();
			if ("NOOP".equals(opName)) continue;

			Pair<String, String> content = entry.content();
			if ("PUT".equals(opName)) {
				kvdb.put(content.getFirst(), content.getSecond());
			} else if ("APPEND".equals(opName)) {
				kvdb.append(content.getFirst(), content.getSecond());
			}

			if (state == State.LEADER) {
				if ("GET".equals(opName)) {
					String value = kvdb.get(content.getFirst());
					send(entry.requester(), new ServerResponseWithContent<>(entry.reqId(), value));
				} else {
					send(entry.requester(), new ServerResponseConfirm(entry.reqId()));
				}
			}
		}
	}

	private void tryAppendToLog(ClientRequestWithContent<?, ?> req) {
		@SuppressWarnings("unchecked")
		var contentMsg = (ClientRequestWithContent<StoreOperationEnums, Pair<String, String>>) req;

		String reqId = req.getRequestId();
		String opName = contentMsg.getOperation().getName();
		Pair<String, String> content = contentMsg.getContent();

		int existingIndex = -1;
		for (int i = log.size() - 1; i >= 0; i--) {
			if (reqId.equals(log.getEntry(i).reqId())) {
				existingIndex = i;
				break;
			}
		}

		if (existingIndex != -1) {
			if (existingIndex <= commitIndex) {
				if ("GET".equals(opName)) {
					send(req.sender, new ServerResponseWithContent<>(reqId, kvdb.get(content.getFirst())));
				} else {
					send(req.sender, new ServerResponseConfirm(reqId));
				}
			}
			return;
		}

		log.append(new LogEntry(currentTerm, opName, content, reqId, req.sender));
		replicateLog();
	}

	private void replicateLog() {
		for (String otherProcess : otherProcessesInCluster) {
			int ni = nextIndex.getOrDefault(otherProcess, 0);

			List<LogEntry> entriesToMsg = new ArrayList<>();
			for (int i = ni; i < log.size(); i++) {
				entriesToMsg.add(log.getEntry(i));
			}

			int prevLogIndex = ni - 1;
			int prevLogTerm = (prevLogIndex < 0) ? 0 : log.getEntry(prevLogIndex).term();
			send(otherProcess, new MsgAppendEntriesReq(currentTerm, prevLogIndex, prevLogTerm, commitIndex, entriesToMsg));
		}
		lastHeartbeat = currentIteration;
	}

	private void startElection() {
		if (state == State.LEADER) return;
		if (currentIteration - lastElection <= electionTimeout) return;

		changeState(State.CANDIDATE, currentTerm + 1);

		int lastLogIndex = log.size() - 1;
		int lastLogTerm = lastLogIndex < 0 ? 0 : log.getLastEntry().term();
		for (String otherProcess : otherProcessesInCluster) {
			send(otherProcess, new MsgVoteReq(currentTerm, lastLogIndex, lastLogTerm));
		}
	}

	private void sendHeartbeats() {
		if (state != State.LEADER) return;
		if (currentIteration - lastHeartbeat <= heartbeatTimeout) return;

		lastHeartbeat = currentIteration;

		for (String otherProcess : otherProcessesInCluster) {
			int ni = nextIndex.getOrDefault(otherProcess, 0);
			int prevLogIndex = ni - 1;
			int prevLogTerm = (prevLogIndex >= 0) ? log.getEntry(prevLogIndex).term() : 0;
			send(otherProcess, new MsgAppendEntriesReq(currentTerm, prevLogIndex, prevLogTerm, commitIndex, List.of()));
		}
	}

	private void handleAppendEntriesRequest(MsgAppendEntriesReq msg) {
		if (currentTerm > msg.getTerm()) {
			send(msg.sender, new MsgAppendEntriesRes(currentTerm, false, log.size()));
			return;
		}

		changeState(State.FOLLOWER, Math.max(currentTerm, msg.getTerm()));
		currentLeader = msg.sender;

		boolean isLogConsistent = (msg.getPrevLogIndex() == -1) || (msg.getPrevLogIndex() < log.size() && log.getEntry(msg.getPrevLogIndex()).term() == msg.getPrevLogTerm());
		if (!isLogConsistent) {
			send(msg.sender, new MsgAppendEntriesRes(currentTerm, false, log.size()));
			return;
		}

		if (!msg.isHeartbeat()) {
			List<LogEntry> entries = msg.getEntries();
			int startIndex = msg.getPrevLogIndex() + 1;
			for (int i = 0; i < entries.size(); i++) {
				int logIndex = startIndex + i;
				if (logIndex < log.size()) {
					if (log.getEntry(logIndex).term() != entries.get(i).term()) {
						log.dropFrom(logIndex);
						log.appendAll(entries.subList(i, entries.size()));
						break;
					}
				} else {
					log.appendAll(entries.subList(i, entries.size()));
					break;
				}
			}
		}

		if (msg.getLeaderCommit() > commitIndex) commitIndex = Math.min(msg.getLeaderCommit(), msg.getPrevLogIndex() + msg.getEntries().size());

		send(msg.sender, new MsgAppendEntriesRes(currentTerm, true, log.size() - 1));
	}

	private void handleAppendEntriesResponse(MsgAppendEntriesRes msg) {
		if (state != State.LEADER || currentTerm > msg.getTerm()) return;

		if (msg.getTerm() > currentTerm) {
			changeState(State.FOLLOWER, msg.getTerm());
			return;
		}

		String followerId = msg.sender;
		int followerIndex = msg.getLastLogIndex();
		if (msg.isSuccess()) {
			if (followerIndex > matchIndex.getOrDefault(followerId, -1)) {
				matchIndex.put(followerId, followerIndex);
				nextIndex.put(followerId, followerIndex + 1);
				updateCommitIndex();
			}
		} else {
			int currentNext = nextIndex.getOrDefault(followerId, 0);
			int nextNext = Math.max(0, Math.min(currentNext - 1, followerIndex));
			nextIndex.put(followerId, nextNext);
			replicateLog();
		}
	}

	private void handleVoteRequest(MsgVoteReq msg) {
		if (currentTerm > msg.getTerm()) {
			send(msg.sender, new MsgVoteRes(currentTerm, false));
			return;
		}

		if (msg.getTerm() > currentTerm) changeState(State.FOLLOWER, msg.getTerm());

		int lastLogIndex = log.size() - 1;
		int lastLogTerm = lastLogIndex < 0 ? 0 : log.getLastEntry().term();
		boolean isLogNew = (msg.getLastLogTerm() > lastLogTerm) || (msg.getLastLogTerm() == lastLogTerm && msg.getLastLogIndex() >= lastLogIndex);
		boolean canVote = (votedFor == null || votedFor.equals(msg.sender));

		if (isLogNew && canVote) {
			votedFor = msg.sender;
			regenerateElectionTimeout();
			send(msg.sender, new MsgVoteRes(currentTerm, true));
		} else {
			send(msg.sender, new MsgVoteRes(currentTerm, false));
		}
	}

	private void handleVoteResponse(MsgVoteRes msg) {
		if (state != State.CANDIDATE || currentTerm > msg.getTerm()) return;

		if (msg.getTerm() > currentTerm) {
			changeState(State.FOLLOWER, msg.getTerm());
			return;
		}

		if (msg.isVoteGranted()) {
			voteCount++;
			if (voteCount >= getQuorum()) {
				changeState(State.LEADER, currentTerm);
				log.append(new LogEntry(currentTerm, "NOOP", null, null, null));
				replicateLog();
			}
		}
	}

	private void handleClientRequest(ClientRequest req) {
		String reqId = req.getRequestId();

		if (state != State.LEADER) {
			send(req.sender, new ServerResponseLeader(reqId, currentLeader));
			return;
		}

		if (req instanceof ClientRequestWhoIsLeader) {
			send(req.sender, new ServerResponseLeader(reqId, currentLeader));
		} else if (req instanceof ClientRequestWithContent<?, ?> clientRequestWithContent) {
			tryAppendToLog(clientRequestWithContent);
		} else {
			System.out.println("Unknown client request: " + req.getClass());
		}
	}
}