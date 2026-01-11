// g++ -o testNN NN.cpp

#include <iostream>
#include <vector>
#include <random>
#include <cassert>

// Generate random weight in range [-1, 1]
double generateRandomWeight() {
    static std::mt19937 generator(std::random_device{}());
    static std::uniform_real_distribution<double> distribution(-1.0, 1.0);
    return distribution(generator);
}

// Class to represent a single Perceptron
class Perceptron {
private:
    std::vector<double> weights; // Weights for inputs
    double bias;                 // Bias term
    double learningRate;         // Learning rate for training

    // Activation function (step function)
    static int activate(double sum) {
        return sum >= 0.0 ? 1 : 0;
    }

public:
    explicit Perceptron(size_t inputSize, double learningRate = 0.01)
        : weights(inputSize), bias(generateRandomWeight()), learningRate(learningRate) {
        for (double& w : weights) {
            w = generateRandomWeight();
        }
    }

    // Forward pass: compute output based on inputs
    int predict(const std::vector<double>& inputs) const {
        assert(inputs.size() == weights.size());

        double sum = bias;
        for (size_t i = 0; i < inputs.size(); ++i) {
            sum += inputs[i] * weights[i];
        }
        return activate(sum);
    }

    // Train the perceptron using the Perceptron Learning Rule
    void train(const std::vector<std::vector<double>>& trainingData,
               const std::vector<int>& labels,
               size_t maxEpochs) {
        assert(trainingData.size() == labels.size());

        for (size_t epoch = 0; epoch < maxEpochs; ++epoch) {
            bool anyError = false;

            for (size_t i = 0; i < trainingData.size(); ++i) {
                assert(trainingData[i].size() == weights.size());

                int prediction = predict(trainingData[i]);
                int error = labels[i] - prediction;

                if (error != 0) {
                    anyError = true;

                    for (size_t j = 0; j < weights.size(); ++j) {
                        weights[j] += learningRate * error * trainingData[i][j];
                    }
                    bias += learningRate * error;
                }
            }

            // Stop early if perfectly classified
            if (!anyError) {
                break;
            }
        }
    }

    // Print weights and bias for debugging
    void printParameters() const {
        std::cout << "Weights: ";
        for (double weight : weights) {
            std::cout << weight << " ";
        }
        std::cout << "\nBias: " << bias << "\n";
    }
};

// Main function for demonstration
int main() {
    // Example: AND gate
    std::vector<std::vector<double>> trainingDataMean = {
        {0.0, 2.0},
        {2.0, 4.0},
        {4.0, 6.0},
        {6.0, 8.0}
    };
    std::vector<int> labelsMean = {1, 3, 5, 7};
    std::vector<std::vector<double>> trainingDataAnd = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {1.0, 1.0}
    };
    std::vector<int> labelsAnd = {0, 0, 0, 1};

    

    Perceptron perceptron(2, 0.1); // 2 inputs, learning rate = 0.1
    perceptron.train(trainingDataMean, labelsMean, 100); // Max 100 epochs

    perceptron.printParameters();

    // Test the perceptron
    std::cout << "\nPredictions:\n";
    for (const auto& inputs : trainingDataMean) {
        std::cout << "Inputs: (" << inputs[0] << ", " << inputs[1]
                  << ") => Output: " << perceptron.predict(inputs) << "\n";
    }

    return 0;
}
