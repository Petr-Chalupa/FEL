package cz.cvut.fel.agents.pdv.student;

import cz.cvut.fel.agents.pdv.dsand.Pair;

import java.io.Serializable;

public record LogEntry(int term, String operation, Pair<String, String> content, String reqId, String requester) implements Serializable {

}
