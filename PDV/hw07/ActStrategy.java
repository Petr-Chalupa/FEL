package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Message;
import cz.cvut.fel.agents.pdv.dsand.Pair;

import java.util.*;


public class ActStrategy {
	static class ProcessStats {
		private final String PID;
		private int lastPingedAt;
		private boolean isDead;

		public ProcessStats(String PID) {
			this.PID = PID;
			this.lastPingedAt = -1;
			this.isDead = false;
		}

		public String getPID() {
			return PID;
		}

		public int getLastPingedAt() {
			return lastPingedAt;
		}

		public void setLastPingedAt(int lastPingedAt) {
			this.lastPingedAt = lastPingedAt;
		}

		public boolean isDead() {
			return isDead;
		}

		public void setDead(boolean isDead) {
			this.isDead = isDead;
		}
	}

	private final Random random = new Random();
	private final int timeoutCycles;
	private int currentCycle;
	private final int maxMsgs;
	private int totalMsgsSent;
	private final Map<String, ProcessStats> pStats;
	private final Map<String, Set<String>> pendingPingReqs; // target, requesters

	public ActStrategy(int maxDelayForMessages, List<String> otherProcesses, int timeToDetectKilledProcess, int upperBoundOnMessages) {
		this.timeoutCycles = (timeToDetectKilledProcess - 4 * maxDelayForMessages) / 2;
		this.currentCycle = 0;
		this.maxMsgs = upperBoundOnMessages;
		this.totalMsgsSent = 0;

		this.pStats = new HashMap<>();
		for (String p : otherProcesses) {
			pStats.put(p, new ProcessStats(p));
		}

		this.pendingPingReqs = new HashMap<>();
	}

	public List<Pair<String, Message>> act(Queue<Message> inbox, String disseminationProcess) {
		currentCycle++;
		List<Pair<String, Message>> outbox = new ArrayList<>();

		// Process incoming messages
		while (!inbox.isEmpty()) {
			Message msg = inbox.poll();
			processMessage(msg, outbox);
		}

		// Check live processes, issue indirect pings or mark as dead
		for (ProcessStats ps : pStats.values()) {
			if (totalMsgsSent >= maxMsgs) break;

			int lastPingedAt = ps.getLastPingedAt();
			int timeFromLastPing = currentCycle - lastPingedAt;

			if (ps.isDead() || lastPingedAt == -1) continue;

			if (timeFromLastPing > 2 * timeoutCycles) {
				ps.setDead(true);
				addMessage(outbox, disseminationProcess, new PFDMessage(ps.getPID()));
			} else if (timeFromLastPing > timeoutCycles) {
				ProcessStats helper = getRandomHelperProcess(ps.getPID());
				if (helper != null) {
					addMessage(outbox, helper.getPID(), new PingMessage(PingMessage.Type.PING_REQ, ps.getPID()));
				}
			}
		}

		// Send random ping to a live process
		if (totalMsgsSent < maxMsgs) {
			ProcessStats randomProcess = getRandomAliveProcess();
			if (randomProcess != null) {
				randomProcess.setLastPingedAt(currentCycle);
				addMessage(outbox, randomProcess.getPID(), new PingMessage(PingMessage.Type.PING));
			}
		}

		return outbox;
	}

	private void processMessage(Message msg, List<Pair<String, Message>> outbox) {
		String sender = msg.sender;

		if (msg instanceof PingMessage ping) {
			PingMessage.Type pingType = ping.getType();

			switch (pingType) {
				case PING: {
					addMessage(outbox, sender, new PingMessage(PingMessage.Type.ACK));
					break;
				}
				case PING_REQ: {
					String target = ping.getTarget();
					if (!pendingPingReqs.containsKey(target)) {
						addMessage(outbox, target, new PingMessage(PingMessage.Type.PING));
					}
					pendingPingReqs.computeIfAbsent(target, k -> new HashSet<>()).add(sender);
					break;
				}
				case ACK: {
					ProcessStats ps = pStats.get(sender);
					if (ps != null) ps.setLastPingedAt(-1);

					Set<String> requesters = pendingPingReqs.remove(sender);
					if (requesters != null) {
						for (String requester : requesters) {
							addMessage(outbox, requester, new PingMessage(PingMessage.Type.PING_REQ_ACK, sender));
						}
					}
					break;
				}
				case PING_REQ_ACK: {
					String target = ping.getTarget();
					ProcessStats ps = pStats.get(target);
					if (ps != null) ps.setLastPingedAt(-1);
					break;
				}
			}
		} else if (msg instanceof DeadProcessMessage dp) {
			String deadProcess = dp.getProcessID();
			ProcessStats ps = pStats.get(deadProcess);
			if (ps != null) ps.setDead(true);
		} else {
			System.out.println("Unknown message: " + msg.getClass() + " from " + sender);
		}
	}

	private void addMessage(List<Pair<String, Message>> outbox, String recipient, Message message) {
		if (totalMsgsSent >= maxMsgs) return;

		outbox.add(new Pair<>(recipient, message));
		totalMsgsSent++;
	}

	private ProcessStats getRandomHelperProcess(String excludePID) {
		List<ProcessStats> helpers = pStats
				.values()
				.stream()
				.filter(ps -> !ps.isDead() && !ps.getPID().equals(excludePID))
				.toList();

		return !helpers.isEmpty() ? helpers.get(random.nextInt(helpers.size())) : null;
	}

	private ProcessStats getRandomAliveProcess() {
		List<ProcessStats> aliveProcesses = pStats
				.values()
				.stream()
				.filter(ps -> !ps.isDead())
				.toList();

		return !aliveProcesses.isEmpty() ? aliveProcesses.get(random.nextInt(aliveProcesses.size())) : null;
	}

}