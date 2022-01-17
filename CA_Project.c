#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#define REGSIZE 8   /* declaring the size of Branch history register globally as it is easier to upadte here is we want to change size of BHR in future.*/
#define TABSIZE 256 /* declaring the size of Pattern History Table globally as it is easier to upadte here is we want to change size of BHR in future.*/

struct bAddressPAP
{
    unsigned int address;
    char outcome;
    char bp;
    char BHR[REGSIZE];
    int PHT[TABSIZE][2];
};
struct fAddressPAP
{
    unsigned int address;
    char BHR[REGSIZE + 1];
    int PHT[TABSIZE][2];
};

int binaryTodecimal(int num)
{
    int rem, decN = 0, base = 1;
    while (num > 0)
    {
        rem = num % 10; /* divide the binary number by 10 and store the remainder in rem variable. */
        decN = decN + rem * base;
        num = num / 10; /* divide the number with quotient */
        base = base * 2;
    }
    return decN;
}
int stateTM(int s1, int s2)
{
    if ((s1 == 1 && s2 == 1) || (s1 == 1 && s2 == 0))
    { /* if Current state is 11 or 10 then prediction is taken(i.e., 1) */
        return 1;
    }
    else
    { /* if Current state is 00 or 01 then prediction is not-taken(i.e., 0) */
        return 0;
    }
}
void updatePHT(int *pht1, int *pht2, char out)
{
    if (out == '+')
    {
        if (*pht1 == 1 && *pht2 == 1)
        { /* moving the state from 11 to 11 as prediction is taken */
            // do nothing
        }
        else
        {
            if (*pht1 == 1 && *pht2 == 0)
            {
                *pht2 = 1; /* moving the state from 10 to 11 as prediction is taken */
            }
            else if (*pht1 == 0 && *pht2 == 1)
            { /* moving the state from 01 to 10 as prediction is taken */
                *pht1 = 1;
                *pht2 = 0;
            }
            else
            { /* moving the state from 00 to 01 as prediction is taken */
                *pht1 = 0;
                *pht2 = 1;
            }
        }
    }
    else if (out == '-')
    {
        if (*pht1 == 0 && *pht2 == 0)
        { /* moving the state from 00 to 00 as prediction is not-taken */
            // do nothing
        }
        else
        {
            if (*pht1 == 0 && *pht2 == 1)
            { /* moving the state from 01 to 00 as prediction is not-taken */
                *pht2 = 0;
            }
            else if (*pht1 == 1 && *pht2 == 0)
            { /* moving the state from 10 to 01 as prediction is not-taken */
                *pht1 = 0;
                *pht2 = 1;
            }
            else
            { /* moving the state from 11 to 10 as prediction is not-taken */
                *pht1 = 1;
                *pht2 = 0;
            }
        }
    }
}
void updateBHR(char bhr[], char pred)
{
    int i;
    for (i = 0; i < REGSIZE - 1; i++)
    {
        bhr[i] = bhr[i + 1]; /* shifting 1 bit left in Branch History Register */
    }
    if (pred == '+')
    {
        bhr[i++] = 1 + '0'; /* Assigning last bit to the BHR based on prediction */
    }
    else
    {
        bhr[i++] = 0 + '0'; /* Assigning last bit to the BHR based on prediction */
    }
}
float accuracyH(int pre, char out, float accuracy, int i)
{
    int outcome;
    if (out == '+')
    {
        outcome = 1;
    }
    else
    {
        outcome = 0;
    }
    if (pre == outcome)
    {
        accuracy = (accuracy + 100); /* every time prediction is correct 100 is added to accuracy. */
    }
    else
    {
        accuracy = (accuracy + 0); /* every time prediction is wrong 0 is added to accuracy. */
    }
    return accuracy;
}
int main()
{

    FILE *fp;
    struct bAddressPAP *bPred;
    struct fAddressPAP *fPred;
    int s1 = 1, s2 = 0;
    int prediction;
    float accuracy = 0;
    bPred = malloc(10 * sizeof(struct bAddressPAP)); /* initially allocating 10 blocks of size */
    fPred = malloc(10 * sizeof(struct fAddressPAP)); /* initially allocating 10 blocks of size to final address structure */
    int tSize = 1;

    fp = fopen("gccSmall.trace", "r");
    if (!fp)
    {
        printf("file not found\n");
        return 1;
    }
    int count = 0;
    int i = 0;

    while (fscanf(fp, "%x %c %c", &bPred[i].address, &bPred[i].outcome, &bPred[i].bp) != EOF) /* Reading trace file line by line */
    {
        int flag = 0;
        for (int k = 0; k < tSize; k++)
        {
            if (fPred[k].address == bPred[i].address)
            {
                printf("Inside %x %x %d\n", fPred[k].address, bPred[i].address, i);
                flag = 1; /* Checking if the address is already present in final address structure */
                break;
            }
        }
        if (flag == 0)
        {
            for (int k = 0; k < REGSIZE; k++)
            {
                fPred[tSize - 1].BHR[k] = 1 + '0';
            }
            for (int k = 0; k < TABSIZE; k++)
            {
                fPred[tSize - 1].PHT[k][0] = 1;
                fPred[tSize - 1].PHT[k][1] = 0;
            }
            fPred[tSize - 1].address = bPred[i].address;
            tSize = tSize + 1;
            s2++;
            if (s2 == 9)
            {
                s2 = 0;
                fPred = realloc(fPred, sizeof(struct fAddressPAP) * (tSize * 10)); /* Reallocating the size of final address struct so that it wouldn't overflow. */
            }
        }
        count++;
        i++;
        s1++;
        if (s1 == 9)
        {
            s1 = 1;
            bPred = realloc(bPred, sizeof(struct bAddressPAP) * (count + 10)); /* Reallocating the size of initial struct to match with the size of trace file */
        }
    }
    /* by now fPred struct contains all the addresses with only one occurence, the size of fPred will be in tSize(which is 1227 for the given trace file) */
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < tSize; j++)
        {
            if (bPred[i].address == fPred[j].address)
            {
                int k = atoi(fPred[j].BHR);
                k = binaryTodecimal(k);                                                /* BHR values is converted to decimal to go to that particular PHT location. */
                prediction = stateTM(fPred[j].PHT[k][0], fPred[j].PHT[k][0]);          /* stateTM returns the predicted value using the current state */
                updatePHT(&fPred[j].PHT[k][0], &fPred[j].PHT[k][1], bPred[i].outcome); /* Pattern History Table is updated */
                updateBHR(fPred[j].BHR, bPred[i].outcome);                             /* Branch History Register is updated */
                accuracy = accuracyH(prediction, bPred[i].outcome, accuracy, i + 1);   /* Every time prediction is correct 100 is added to accuracy and 0 if prediction is wrong. */
            }
        }
    }

    printf("\nFinal Accuracy : %f", (accuracy / i)); /* accuracy contains (number of times prediction is correct)*100, then if we divide it with i(which is total number of inputs in the given trace file) gives the final accuracy of the algorithm. */
    fclose(fp);
    return 0;
}
