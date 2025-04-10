package cz.cvut.fel.pjv;

public class BruteForceAttacker extends Thief {
    @Override
    public void breakPassword(int sizeOfPassword) {
        solve(getCharacters(), new char[sizeOfPassword], 0);
    }

    private void solve(char[] charset, char[] pwd, int size) {
        if (isOpened()) {
            return;
        }
        if (size == pwd.length) {
            tryOpen(pwd);
            return;
        }

        for (char c : charset) {
            pwd[size] = c;
            solve(charset, pwd, size + 1);
        }
    }
}
