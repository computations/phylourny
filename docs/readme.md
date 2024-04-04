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

# Downloading and Building

To download Phylourny, use the command

```
git clone --recursive https://github.com/computations/phylourny
```

and to build

```
make
```

The resulting binaries will be placed in `bin`. Depending on the packages installed locally, there might be 3 binaries:

- `phylourny`: The main binary.
- `phylourny_test`: The test suite.
- `phylourny_bench`: The benchmarking suite.

For most people, only the main binary will be interesting. The test binary will be useful if one needs to check the code
does actually work, but the benchmarking binary contains a lot of "extra" benchmarks that are not _really_ that relevant
to the program.

The test suite will be built if CMake detects that the address sanitizer is available with the compiler used, and if the
option `PHYLOURNY_BUILD_TESTS` is set, which it is by default. To disable building tests, set the option
`PHYLOURNY_BUILD_TESTS` to `OFF`. 

Benchmarks will be built if the flag `PHYLOURNY_BUILD_BENCH` is set on, which it is by default. To disable building the
benchmark suite, set `PHYLOURNY_BUILD_BENCH` to `OFF`.

# Examples

There are example datasets in the `experiments` directory. These are a good
place to start if you want to stary using the tool.
