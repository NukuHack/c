#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double sigmoid(double z)
{
    return 1.0 / (1.0 + exp(-z));
}

int main(void)
{
    double X[4][2] = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {1.0, 1.0}
    };
    double y[4] = {0.0, 1.0, 1.0, 0.0};

    double W1[2][2];
    double b1[2];
    double W2[2];
    double b2;

    double dW1[2][2];
    double db1[2];
    double dW2[2];
    double db2;

    double z1[2], a1[2], z2, yhat;
    double da1[2], dz1[2], dz2;
    double loss, eps;
    double lr;
    int epochs;
    int i, j, s, epoch;

    srand(1);

    /* Initialize weights */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            W1[i][j] = ((double)rand() / (double)RAND_MAX - 0.5) * 0.5;
        }
    }

    for (j = 0; j < 2; j++) {
        b1[j] = 0.0;
        W2[j] = ((double)rand() / (double)RAND_MAX - 0.5) * 0.5;
    }

    b2 = 0.0;

    lr = 0.5;
    epochs = 20000;
    eps = 1e-9;

    for (epoch = 0; epoch < epochs; epoch++) {
        /* Zero gradients */
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                dW1[i][j] = 0.0;
            }
            db1[i] = 0.0;
        }

        for (j = 0; j < 2; j++) {
            dW2[j] = 0.0;
        }
        db2 = 0.0;

        loss = 0.0;

        /* Forward + backward for each sample */
        for (s = 0; s < 4; s++) {
            /* Forward pass */
            for (j = 0; j < 2; j++) {
                z1[j] = b1[j] + X[s][0] * W1[0][j] + X[s][1] * W1[1][j];
                a1[j] = sigmoid(z1[j]);
            }

            z2 = b2 + a1[0] * W2[0] + a1[1] * W2[1];
            yhat = sigmoid(z2);

            /* Binary cross-entropy loss */
            loss += -(y[s] * log(yhat + eps) +
                      (1.0 - y[s]) * log(1.0 - yhat + eps));

            /* Backward pass */
            dz2 = yhat - y[s];

            for (j = 0; j < 2; j++) {
                dW2[j] += dz2 * a1[j];
            }
            db2 += dz2;

            for (j = 0; j < 2; j++) {
                da1[j] = dz2 * W2[j];
                dz1[j] = da1[j] * a1[j] * (1.0 - a1[j]);
            }

            for (i = 0; i < 2; i++) {
                for (j = 0; j < 2; j++) {
                    dW1[i][j] += X[s][i] * dz1[j];
                }
            }

            for (j = 0; j < 2; j++) {
                db1[j] += dz1[j];
            }
        }

        loss /= 4.0;

        /* Gradient descent update, dividing by batch size */
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                W1[i][j] -= lr * dW1[i][j] / 4.0;
            }
        }

        for (j = 0; j < 2; j++) {
            b1[j] -= lr * db1[j] / 4.0;
            W2[j] -= lr * dW2[j] / 4.0;
        }

        b2 -= lr * db2 / 4.0;

        if (epoch % 2000 == 0) {
            printf("epoch %5d  loss %f\n", epoch, loss);
        }
    }

    printf("\nPredictions:\n");

    for (s = 0; s < 4; s++) {
        for (j = 0; j < 2; j++) {
            z1[j] = b1[j] + X[s][0] * W1[0][j] + X[s][1] * W1[1][j];
            a1[j] = sigmoid(z1[j]);
        }

        z2 = b2 + a1[0] * W2[0] + a1[1] * W2[1];
        yhat = sigmoid(z2);

        printf("%.0f %.0f -> %.4f\n", X[s][0], X[s][1], yhat);
    }

    return 0;
}