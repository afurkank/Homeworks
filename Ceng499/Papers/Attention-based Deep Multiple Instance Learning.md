# Attention-based Deep Multiple Instance Learning

### Objective
The primary goal of this research is to enhance the Multiple Instance Learning (MIL) approach in supervised learning, where a single class label is assigned to a bag of instances. The paper aims to address the challenge of learning the Bernoulli distribution of the bag label, fully parameterized by neural networks, and to introduce a neural network-based permutation-invariant aggregation operator corresponding to the attention mechanism. This approach is designed to improve interpretability and performance in MIL tasks, particularly in areas like medical imaging, where interpretability is crucial.

### Background
MIL is a variant of supervised learning applicable in scenarios where multiple instances are observed, but only a general category label is provided. This is common in medical imaging, where a diagnosis is often based on a collection of image instances (a bag), each contributing to the overall diagnosis. Traditional MIL methods suffer from low instance-level accuracy and lack interpretability, making them less suitable for practical applications where understanding the contribution of each instance to the final decision is vital.

### Methodologies
#### 1. Problem Formulation and MIL Approaches:
- **MIL Problem Definition**: The problem is formulated as learning a model that predicts a bag label (e.g., a medical diagnosis) based on a collection of instances. The model must be permutation-invariant, meaning the order of instances in the bag should not affect the prediction.
- **MIL Pooling**: Different MIL pooling strategies are discussed, with the aim to aggregate instance information into a single bag representation. Common operators include maximum, mean, and others like noisy-or. The paper highlights the limitation of these pre-defined, non-trainable operators.

#### 2. Neural Network Integration:
- **Neural Network Parameterization**: The paper proposes using neural networks to parameterize the transformations involved in MIL. This approach allows for end-to-end training and enhanced flexibility in modeling permutation-invariant score functions.
- **Instance and Embedding Approaches**: Two primary approaches are highlighted - instance-based and embedding-based. The embedding-based approach, where instances are transformed to a low-dimensional space and then aggregated, is advocated for better bag-level classification performance.

#### 3. Attention-based MIL Pooling:
- **Introduction of Attention Mechanism**: The key contribution of the paper is the introduction of an attention-based MIL pooling mechanism. This approach uses a weighted average of instance embeddings, with weights determined by a neural network. It aims to be more flexible and interpretable than traditional MIL pooling operators.
- **Gated Attention Mechanism**: To address potential limitations in learning complex relations with the attention mechanism, a gated attention mechanism is introduced. It combines a hyperbolic tangent non-linearity with a gating mechanism, enhancing the model's ability to learn nuanced relationships between instances.
- **Interpretability and Practical Applications**: The attention mechanism is designed to make the model's decisions more interpretable. In medical applications, for example, it can help identify key instances or regions of interest (ROIs) that contribute significantly to the diagnosis.

### Conclusion
The paper demonstrates that the proposed attention-based deep MIL approach achieves comparable or superior performance to existing MIL methods on various datasets, including medical imaging tasks. It emphasizes the importance of interpretability in practical applications and suggests future research directions, such as exploring multi-class MIL problems and considering dependencies among instances within a bag.

In summary, the paper presents a significant advancement in MIL by introducing an attention mechanism, enhancing the model's flexibility, interpretability, and performance in tasks where understanding the contribution of individual instances is crucial.

## Paper link
https://arxiv.org/abs/1802.04712
