/* loan_repayment.c
   Compile: gcc loan_repayment.c -o loan_repayment
*/
#include <stdio.h>

double calculateRepayment(double loan, double rate, int years, int currentYear, double installment, double extraPayment) {
    if (loan <= 0.000001 || years == 0) { // base case: loan repaid or no years left
        if (loan <= 0.000001) printf("Loan fully repaid before or at year %d.\n", currentYear-1);
        return 0.0;
    }

    // pay installment + optional extraPayment this year (principal portion)
    double principalPaid = installment + extraPayment;
    if (principalPaid > loan) principalPaid = loan; // don't overpay principal

    double remainingAfterPrincipal = loan - principalPaid;
    // interest is charged on remaining principal for that year
    double interest = remainingAfterPrincipal * rate;
    double totalThisYear = principalPaid + interest;

    printf("Year %d: Principal paid=%.2f, Interest=%.2f, Remaining loan after payment=%.2f\n",
           currentYear, principalPaid, interest, remainingAfterPrincipal);

    // recursive call for remaining years
    return totalThisYear + calculateRepayment(remainingAfterPrincipal, rate, years-1, currentYear+1, installment, extraPayment);
}

int main() {
    double loan = 100000.0;
    double rate = 0.05; // 5% yearly
    int years = 3;

    // equal principal installment each year
    double installment = loan / years;
    double extraPayment = 0.0; // extension: set >0 to add extra yearly payment

    printf("Loan = %.2f, Rate = %.2f%%, Years = %d\n", loan, rate*100.0, years);
    printf("Equal principal installment per year = %.2f\n\n", installment);

    double total = calculateRepayment(loan, rate, years, 1, installment, extraPayment);
    printf("\nTotal repayment over %d years = %.2f\n", years, total);

    return 0;
}
