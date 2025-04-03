double generateRandomWeight() {
    return static_cast<double>(rand()) / RAND_MAX * 2.0 - 1.0; // Range: [-1, 1]
}

// Class to represent a single Perceptron
class Perceptron {
private:
    std::vector<double> weights; // Weights for inputs
    double bias;                // Bias term
    double learningRate;        // Learning rate for training

public:
    Perceptron(size_t inputSize, double learningRate = 0.01) : learningRate(learningRate) {
        // Initialize weights and bias randomly
        weights.resize(inputSize);
        for (size_t i = 0; i < inputSize; ++i) {
            weights[i] = generateRandomWeight();
        }
        bias = generateRandomWeight();
    }

    // Activation function (step function)
    int activate(double sum) {
        return sum >= 0.0 ? 1 : 0;
    }

    // Forward pass: compute output based on inputs
    int predict(const std::vector<double>& inputs) {
        double sum = bias;
        for (size_t i = 0; i < inputs.size(); ++i) {
            sum += inputs[i] * weights[i];
        }
        return activate(sum);
    }

    // Train the perceptron using the Perceptron Learning Rule
    void train(const std::vector<std::vector<double>>& trainingData,
               const std::vector<int>& labels,
               size_t epochs) {
        for (size_t epoch = 0; epoch < epochs; ++epoch) {
            for (size_t i = 0; i < trainingData.size(); ++i) {
                int prediction = predict(trainingData[i]);
                int error = labels[i] - prediction;

                // Update weights and bias based on error
                for (size_t j = 0; j < weights.size(); ++j) {
                    weights[j] += learningRate * error * trainingData[i][j];
                }
                bias += learningRate * error;
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
    srand(static_cast<unsigned>(time(0))); // Seed for random number generation

    // Example: AND gate
    std::vector<std::vector<double>> trainingData = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    std::vector<int> labels = {0, 0, 0, 1};

    Perceptron perceptron(2, 0.1); // 2 inputs, learning rate = 0.1
    perceptron.train(trainingData, labels, 10); // Train for 10 epochs

    perceptron.printParameters();

    // Test the perceptron
    std::cout << "Predictions:\n";
    for (const auto& inputs : trainingData) {
        std::cout << "Inputs: (" << inputs[0] << ", " << inputs[1] << ") => Output: "
                  << perceptron.predict(inputs) << "\n";
    }

    return 0;
}

