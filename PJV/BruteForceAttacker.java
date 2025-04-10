package cz.cvut.fel.pjv;

import java.util.Arrays;

public class BruteForceAttacker {
    private char[] characters;
    private char[] password;
    private boolean initialized = false;
    private boolean opened = false;

    public char[] getCharacters() {
        return characters;
    }

    public boolean isOpened() {
        return opened;
    }

    public final void init(char[] charactersArray, String passwordString) {
        if (initialized) {
            System.err.println("Function init has already been called");
            return;
        }
        initialized = true;

        Arrays.sort(charactersArray);

        characters = Arrays.copyOf(charactersArray, charactersArray.length);
        password = passwordString.toCharArray();
    }

    public void breakPassword(int sizeOfPassword) {
        solve(characters, new char[sizeOfPassword], 0);
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

    public boolean tryOpen(char[] input) {
        if (password.length != input.length) {
            System.err.println("Given password must be of the "
                    + "same size as password of the vault (" + password.length
                    + "), but was: " + input.length);
            return false;
        }

        opened = Arrays.equals(password, input);

        if (opened) {
            System.out.println("...click!...");
        }

        return opened;
    }
}
