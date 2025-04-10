package cz.cvut.fel.pjv;

import java.util.Scanner;

public class Calculator {
    private static final String[][] operandNames = {
            { "scitanec", "scitanec" },
            { "mensenec", "mensitel" },
            { "cinitel", "cinitel" },
            { "delenec", "delitel" }
    };
    private static final char[] operatorSigns = { '+', '-', '*', '/' };

    public void homework() {
        try (Scanner sc = new Scanner(System.in)) {
            int operator;
            double operand1;
            double operand2;
            int precision;

            System.out.println("Vyber operaci (1-soucet, 2-rozdil, 3-soucin, 4-podil):");
            operator = sc.nextInt();
            if (operator < 1 || operator > 4) {
                System.out.println("Chybna volba!");
                return;
            }

            System.out.println("Zadej " + operandNames[operator - 1][0] + ": ");
            operand1 = sc.nextDouble();

            System.out.println("Zadej " + operandNames[operator - 1][1] + ": ");
            operand2 = sc.nextDouble();
            if (operator == 4 && operand2 == 0) {
                System.out.println("Pokus o deleni nulou!");
                return;
            }

            System.out.println("Zadej pocet desetinnych mist: ");
            precision = sc.nextInt();
            if (precision < 0) {
                System.out.println("Chyba - musi byt zadane kladne cislo!");
                return;
            }

            double result = 0;
            switch (operator) {
                case 1:
                    result = operand1 + operand2;
                    break;
                case 2:
                    result = operand1 - operand2;
                    break;
                case 3:
                    result = operand1 * operand2;
                    break;
                case 4:
                    result = operand1 / operand2;
                    break;
            }

            String resStr = "%." + precision + "f %c %." + precision + "f = %." + precision + "f";
            System.out.format(resStr, operand1, operatorSigns[operator - 1], operand2, result);
            System.out.println();
        }
    }

}