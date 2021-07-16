# Phylourny

Phylourny is a toy project to apply a modified version of Felsenstein's pruning
algorithm to predicting tournament, such as football or basketball tournaments.
The big advantage of doing this, over just running simulations, is that we
obtain higher fidelity estimations with less effort. 

# The Algorithm

As stated before the algorithm used in Phylourny is a modified version of
Felsenstein's pruning algorithm for evaluating the likelihood of a tree and a
model given some data. As pointed out by Zhang in his book _Computational
Molecular Evolution_ this algorithm is itself a version of a method from the
early Chinese mathematics to evaluate polynomials faster. We use this to
enumerate the paths a team can take through the tournament in a quicker fashion
than brute force enumeration.

It is probably important to note that this problem is unlikely to get to the
size that this is needed. Most tournaments in the world are small, and are
perfectly capable of being sampled, or of enumerating the paths explicitly.
