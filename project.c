#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

struct BranchPrediction
{
    unsigned int address;
    char outcome;
    char secondaryOutcome;
};

struct UniqueAddress
{
    unsigned int address;
    char BHR[8];
    int PHT[256][2];
};

int binaryTodecimal(int binary)
{
    int remainder, decimal = 0, base = 1;
    while (binary > 0)
    {
        remainder = binary % 10;
        decimal = decimal + remainder * base;
        binary = binary / 10;
        base = base * 2;
    }
    return decimal;
}

int stateMachine(int state1, int state2)
{
    if ((state1 == 1 && state2 == 1) || (state1 == 1 && state2 == 0))
        return 1;
    return 0;
}

void updatePHT(int *pht1, int *pht2, char outcome)
{
    if (outcome == '+')
    {
        if (*pht1 == 1 && *pht2 == 0)
        {
            *pht2 = 1;
        }
        else if (*pht1 == 0 && *pht2 == 1)
        {
            *pht1 = 1;
            *pht2 = 0;
        }
        else
        {
            *pht1 = 0;
            *pht2 = 1;
        }
    }
    if (outcome == '-')
    {
        if (*pht1 == 0 && *pht2 == 1)
        {
            *pht2 = 0;
        }
        else if (*pht1 == 1 && *pht2 == 0)
        {
            *pht1 = 0;
            *pht2 = 1;
        }
        else
        {
            *pht1 = 1;
            *pht2 = 0;
        }
    }
}

void shiftBHR(char bhr[], int pred)
{
    int i;
    for (i = 0; i < 7; i++)
    {
        bhr[i] = bhr[i + 1];
    }
    if (pred == '+')
    {
        bhr[i++] = 1 + '0';
    }
    else
    {
        bhr[i++] = 0 + '0';
    }
}

int main()
{
    FILE *fp;
    struct BranchPrediction *bpred;
    struct UniqueAddress *uniqAddr;
    int prediction;
    bpred = malloc(sizeof(struct BranchPrediction));
    uniqAddr = malloc(sizeof(struct UniqueAddress));
    long int uniqAddrSize = 0;
    long int size = 0;
    fp = fopen("gccSmall.trace", "r");
    if (!fp)
    {
        printf("File not found\n");
        return 1;
    }
    int j;
    long int predPercentage = 0;
    while (fscanf(fp, "%x %c %c", &bpred[0].address, &bpred[0].outcome, &bpred[0].secondaryOutcome) != EOF)
    {
        int k, position = 0;
        for (j = 0; j < uniqAddrSize; j++)
        {
            if (uniqAddr[j].address == bpred[0].address)
            {
                position = j;
                break;
            }
        }
        if (position == 0)
        {
            uniqAddr[uniqAddrSize].address = bpred[0].address;
            for (j = 0; j < 8; j++)
            {
                uniqAddr[uniqAddrSize].BHR[j] = 1 + '0';
            }
            for (j = 0; j < 256; j++)
            {
                uniqAddr[uniqAddrSize].PHT[j][0] = 1;
                uniqAddr[uniqAddrSize].PHT[j][1] = 0;
            }
            position = uniqAddrSize;
            uniqAddrSize++;
            uniqAddr = realloc(uniqAddr, sizeof(struct UniqueAddress) * (uniqAddrSize + 1));
        }

        k = atoi(uniqAddr[position].BHR);
        k = binaryTodecimal(k);
        prediction = stateMachine(uniqAddr[position].PHT[k][0], uniqAddr[position].PHT[k][1]);
        updatePHT(&uniqAddr[position].PHT[k][0], &uniqAddr[position].PHT[k][1], bpred[0].outcome);
        shiftBHR(uniqAddr[position].BHR, prediction);
        if (bpred[0].outcome == bpred[0].secondaryOutcome)
            predPercentage = predPercentage + 1;

        size++;
        // printf("End of program. %f %d\n", predPercentage, size);
    }
    printf("Prediction percentage is %f (Successfull predictions: %ld out of: %ld)\n", (predPercentage / (float)size) * 100, predPercentage, size);

    fclose(fp);
    return 0;
}
