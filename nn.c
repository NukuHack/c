#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "rand.h"

#define N_IN     8
#define N_HID    128
#define N_DIGITS 3
#define N_CLASS  10
#define N_OUT    (N_DIGITS * N_CLASS)   // 30

// Use float for speed
static float W1[N_HID * N_IN], b1[N_HID];
static float W2T[N_HID * N_OUT], b2[N_OUT];   // transposed: W2T[i*N_OUT + k] = W2[k][i]

// Adam moments
static float mW1[N_HID * N_IN], vW1[N_HID * N_IN];
static float mb1[N_HID], vb1[N_HID];
static float mW2T[N_HID * N_OUT], vW2T[N_HID * N_OUT];
static float mb2[N_OUT], vb2[N_OUT];

static float inputs[256][N_IN];
static int targets[256][N_DIGITS];

void init_data(void) {
    for (int n = 0; n < 256; n++) {
        for (int b = 0; b < N_IN; b++)
            inputs[n][b] = (float)((n >> (7 - b)) & 1);
        targets[n][0] = n / 100;
        targets[n][1] = (n / 10) % 10;
        targets[n][2] = n % 10;
    }
}

void init_weights(void) {
    seed_rng((uint64_t)time(NULL));
    float std1 = sqrtf(2.0f / N_IN);   // He for ReLU
    float std2 = sqrtf(2.0f / N_HID);

    for (int i = 0; i < N_HID; i++) {
        for (int j = 0; j < N_IN; j++)
            W1[i * N_IN + j] = (float)randn() * std1;
        b1[i] = 0.0f;
    }
    for (int k = 0; k < N_OUT; k++) {
        for (int i = 0; i < N_HID; i++)
            W2T[i * N_OUT + k] = (float)randn() * std2;
        b2[k] = 0.0f;
    }
}

// Forward + backward for one sample, accumulating gradients.
// If loss_out != NULL, the loss is also computed.
static inline void forward_backward(int n, float *dW1, float *db1,
                                    float *dW2T, float *db2,
                                    float *loss_out) {
    float a1[N_HID];
    const float *x = inputs[n];

    // Hidden layer
    for (int i = 0; i < N_HID; i++) {
        float z = b1[i];
        const float *w = &W1[i * N_IN];
        for (int j = 0; j < N_IN; j++) z += w[j] * x[j];
        a1[i] = z > 0.0f ? z : 0.0f;
    }

    float delta_out[N_OUT];
    float loss = 0.0f;

    // For each digit
    for (int d = 0; d < N_DIGITS; d++) {
        float z[N_CLASS];
        int base_k = d * N_CLASS;
        for (int c = 0; c < N_CLASS; c++) z[c] = b2[base_k + c];

        // Compute logits using transposed weights (cache friendly)
        for (int i = 0; i < N_HID; i++) {
            float a = a1[i];
            const float *w = &W2T[i * N_OUT + base_k];
            for (int c = 0; c < N_CLASS; c++) {
                z[c] += w[c] * a;
            }
        }

        // Softmax
        float max_z = z[0];
        for (int c = 1; c < N_CLASS; c++)
            if (z[c] > max_z) max_z = z[c];
        float sum = 0.0f;
        for (int c = 0; c < N_CLASS; c++) {
            z[c] = expf(z[c] - max_z);
            sum += z[c];
        }
        for (int c = 0; c < N_CLASS; c++) {
            int k = base_k + c;
            float prob = z[c] / sum;
            delta_out[k] = prob - (c == targets[n][d] ? 1.0f : 0.0f);
            if (loss_out && c == targets[n][d])
                loss -= logf(prob + 1e-12f);
        }
    }
    if (loss_out) *loss_out = loss;

    // Accumulate output gradients
    for (int k = 0; k < N_OUT; k++) {
        db2[k] += delta_out[k];
    }
    for (int i = 0; i < N_HID; i++) {
        float a = a1[i];
        float *dW = &dW2T[i * N_OUT];
        for (int k = 0; k < N_OUT; k++) {
            dW[k] += a * delta_out[k];
        }
    }

    // Backprop to hidden
    float delta_h[N_HID];
    for (int i = 0; i < N_HID; i++) {
        float sum = 0.0f;
        const float *w = &W2T[i * N_OUT];
        for (int k = 0; k < N_OUT; k++) {
            sum += delta_out[k] * w[k];
        }
        delta_h[i] = (a1[i] > 0.0f) ? sum : 0.0f;
    }

    // Accumulate hidden gradients
    for (int i = 0; i < N_HID; i++) {
        float delta = delta_h[i];
        db1[i] += delta;
        float *dW = &dW1[i * N_IN];
        for (int j = 0; j < N_IN; j++) {
            dW[j] += delta * x[j];
        }
    }
}

void train(void) {
    const float lr = 0.001f;
    const float beta1 = 0.9f, beta2 = 0.999f, eps = 1e-8f;
    const int epochs = 5000;
    const int batch_size = 256;
    int t = 0;
    float beta1_pow = 1.0f, beta2_pow = 1.0f;

    float dW1[N_HID * N_IN];
    float db1[N_HID];
    float dW2T[N_HID * N_OUT];
    float db2[N_OUT];

    for (int epoch = 0; epoch <= epochs; epoch++) {
        memset(dW1, 0, sizeof(dW1));
        memset(db1, 0, sizeof(db1));
        memset(dW2T, 0, sizeof(dW2T));
        memset(db2, 0, sizeof(db2));

        float total_loss = 0.0f;
        int print = (epoch % 500 == 0);

        for (int n = 0; n < 256; n++) {
            float loss;
            forward_backward(n, dW1, db1, dW2T, db2, print ? &loss : NULL);
            if (print) total_loss += loss;
        }

        t++;
        beta1_pow *= beta1;
        beta2_pow *= beta2;
        float bc1 = 1.0f - beta1_pow;
        float bc2 = 1.0f - beta2_pow;
        float scale = 1.0f / batch_size;

        // Adam update for W1, b1
        for (int i = 0; i < N_HID * N_IN; i++) {
            float g = dW1[i] * scale;
            mW1[i] = beta1 * mW1[i] + (1 - beta1) * g;
            vW1[i] = beta2 * vW1[i] + (1 - beta2) * g * g;
            float m_hat = mW1[i] / bc1;
            float v_hat = vW1[i] / bc2;
            W1[i] -= lr * m_hat / (sqrtf(v_hat) + eps);
        }
        for (int i = 0; i < N_HID; i++) {
            float g = db1[i] * scale;
            mb1[i] = beta1 * mb1[i] + (1 - beta1) * g;
            vb1[i] = beta2 * vb1[i] + (1 - beta2) * g * g;
            float m_hat = mb1[i] / bc1;
            float v_hat = vb1[i] / bc2;
            b1[i] -= lr * m_hat / (sqrtf(v_hat) + eps);
        }
        // Adam update for W2T, b2
        for (int i = 0; i < N_HID * N_OUT; i++) {
            float g = dW2T[i] * scale;
            mW2T[i] = beta1 * mW2T[i] + (1 - beta1) * g;
            vW2T[i] = beta2 * vW2T[i] + (1 - beta2) * g * g;
            float m_hat = mW2T[i] / bc1;
            float v_hat = vW2T[i] / bc2;
            W2T[i] -= lr * m_hat / (sqrtf(v_hat) + eps);
        }
        for (int k = 0; k < N_OUT; k++) {
            float g = db2[k] * scale;
            mb2[k] = beta1 * mb2[k] + (1 - beta1) * g;
            vb2[k] = beta2 * vb2[k] + (1 - beta2) * g * g;
            float m_hat = mb2[k] / bc1;
            float v_hat = vb2[k] / bc2;
            b2[k] -= lr * m_hat / (sqrtf(v_hat) + eps);
        }

        if (print) {
            printf("Epoch %5d, loss = %.6f\n", epoch, total_loss / 256.0f);
            int test_vals[] = {0, 1, 42, 127, 255};
            for (int i = 0; i < 5; i++) {
                int n = test_vals[i];
                // Simple forward for prediction
                float a1[N_HID];
                const float *x = inputs[n];
                for (int j = 0; j < N_HID; j++) {
                    float z = b1[j];
                    const float *w = &W1[j * N_IN];
                    for (int k = 0; k < N_IN; k++) z += w[k] * x[k];
                    a1[j] = z > 0.0f ? z : 0.0f;
                }
                int pred[N_DIGITS];
                for (int d = 0; d < N_DIGITS; d++) {
                    float z[N_CLASS];
                    int base_k = d * N_CLASS;
                    for (int c = 0; c < N_CLASS; c++) z[c] = b2[base_k + c];
                    for (int j = 0; j < N_HID; j++) {
                        float a = a1[j];
                        const float *w = &W2T[j * N_OUT + base_k];
                        for (int c = 0; c < N_CLASS; c++) z[c] += w[c] * a;
                    }
                    int max_c = 0;
                    float max_z = z[0];
                    for (int c = 1; c < N_CLASS; c++)
                        if (z[c] > max_z) { max_z = z[c]; max_c = c; }
                    pred[d] = max_c;
                }
                printf("  %3d -> %d%d%d (target %d%d%d)\n",
                       n, pred[0], pred[1], pred[2],
                       targets[n][0], targets[n][1], targets[n][2]);
            }
        }
    }
}

void predict(int n, int pred[N_DIGITS]) {
    float a1[N_HID];
    const float *x = inputs[n];
    for (int i = 0; i < N_HID; i++) {
        float z = b1[i];
        const float *w = &W1[i * N_IN];
        for (int j = 0; j < N_IN; j++) z += w[j] * x[j];
        a1[i] = z > 0.0f ? z : 0.0f;
    }
    for (int d = 0; d < N_DIGITS; d++) {
        float z[N_CLASS];
        int base_k = d * N_CLASS;
        for (int c = 0; c < N_CLASS; c++) z[c] = b2[base_k + c];
        for (int i = 0; i < N_HID; i++) {
            float a = a1[i];
            const float *w = &W2T[i * N_OUT + base_k];
            for (int c = 0; c < N_CLASS; c++) z[c] += w[c] * a;
        }
        int max_c = 0;
        float max_z = z[0];
        for (int c = 1; c < N_CLASS; c++)
            if (z[c] > max_z) { max_z = z[c]; max_c = c; }
        pred[d] = max_c;
    }
}

int main(void) {
    init_data();
    init_weights();
    train();

    int correct = 0;
    for (int n = 0; n < 256; n++) {
        int pred[N_DIGITS];
        predict(n, pred);
        if (pred[0] == targets[n][0] &&
            pred[1] == targets[n][1] &&
            pred[2] == targets[n][2])
            correct++;
        else
            printf("Mismatch: %3d -> %d%d%d (target %d%d%d)\n",
                   n, pred[0], pred[1], pred[2],
                   targets[n][0], targets[n][1], targets[n][2]);
    }
    printf("\nFinal accuracy: %d/256 (%.2f%%)\n", correct, correct * 100.0 / 256.0);

    printf("\nEnter a number 0-255 to test (-1 to quit):\n");
    int n;
    while (scanf("%d", &n) == 1 && n >= 0) {
        if (n > 255) { printf("Out of range.\n"); continue; }
        int pred[N_DIGITS];
        predict(n, pred);
        printf("Predicted: %d%d%d (target %d%d%d)\n",
               pred[0], pred[1], pred[2],
               targets[n][0], targets[n][1], targets[n][2]);
    }
    return 0;
}
