package cz.cvut.fel.agents.pdv.exclusion;

import cz.cvut.fel.agents.pdv.clocked.ClockedMessage;
import cz.cvut.fel.agents.pdv.clocked.ClockedProcess;

import java.util.ArrayDeque;
import java.util.HashSet;
import java.util.Queue;
import java.util.Set;

public class ExclusionPrimitive {
	enum AcquisitionState {
		RELEASED,
		WANTED,
		HELD
	}

	private final ClockedProcess owner;
	private final String criticalSectionName;
	private final String[] allAccessingProcesses;
	private AcquisitionState state;
	private int requestTimestamp;
	Queue<String> deferredRequests = new ArrayDeque<>();
	Set<String> pendingOkReplies = new HashSet<>();

	public ExclusionPrimitive(ClockedProcess owner, String criticalSectionName, String[] allProcesses) {
		this.owner = owner;
		this.criticalSectionName = criticalSectionName;
		this.allAccessingProcesses = allProcesses;
		this.state = AcquisitionState.RELEASED;
	}

	public boolean accept(ClockedMessage m) {
		if (!(m instanceof ExclusionMsg msg)) return false;
		if (!criticalSectionName.equals(msg.getCriticalSectionName())) return false;

		String sender = msg.sender;
		if (msg.getType() == ExclusionMsg.Type.REQUEST) {
			if (state == AcquisitionState.HELD || (state == AcquisitionState.WANTED && compareLogicalTime(requestTimestamp, owner.id, m.sentOn, sender))) {
				deferredRequests.add(sender);
			} else {
				owner.increaseTime();
				owner.send(sender, new ExclusionMsg(criticalSectionName, ExclusionMsg.Type.OK));
			}
		} else if (msg.getType() == ExclusionMsg.Type.OK) {
			pendingOkReplies.remove(sender);
			if (pendingOkReplies.isEmpty()) state = AcquisitionState.HELD;
		}

		return true;
	}

	public void requestEnter() {
		owner.increaseTime();
		requestTimestamp = owner.getTime();
		this.state = AcquisitionState.WANTED;

		pendingOkReplies = new HashSet<>();
		for (String processId : allAccessingProcesses) {
			if (processId.equals(owner.id)) continue;

			pendingOkReplies.add(processId);
			owner.send(processId, new ExclusionMsg(criticalSectionName, ExclusionMsg.Type.REQUEST));
		}
	}

	public void exit() {
		this.state = AcquisitionState.RELEASED;

		while (!deferredRequests.isEmpty()) {
			String processId = deferredRequests.poll();
			owner.increaseTime();
			owner.send(processId, new ExclusionMsg(criticalSectionName, ExclusionMsg.Type.OK));
		}
	}

	public String getName() {
		return criticalSectionName;
	}

	public boolean isHeld() {
		return state == AcquisitionState.HELD;
	}

	private boolean compareLogicalTime(int currentTime, String currentId, int otherTime, String otherId) {
		return (currentTime < otherTime) || (currentTime == otherTime && currentId.compareTo(otherId) < 0);
	}
}