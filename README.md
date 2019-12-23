# Relude Random

Generate pseudo-random values in ReasonML (in a way that is reproducible and free from side effects). This library implements [this paper](http://www.pcg-random.org/paper.html), and is largely a port of [elm/random](https://package.elm-lang.org/packages/elm-community/random-extra/latest).

## Installation

**Install from npm**

```sh
npm install --save relude-random
```

Note that `relude-random` has peer-dependencies on `relude` and `bs-abstract`, both of which can also be instlled from npm if your project doesn't already use them.

**Update your bsconfig.json**

```json
{
  "dependencies": [
    "bs-abstract",
    "relude",
    "relude-random"
  ],
  // ...
}
```

## Understanding Relude Random

If the goal of pure, functional programming is to write functions that always produce the same output, given the same input, without producing any observable side effects, this would seem to be at odds with randomness.

Instead of _true_ randomness, we opt for _generators_ that can be run with a _seed_. Running the same generator with the same seed will always produce the same value. This is "good enough" for many situations where randomness is needed (such as games) because:

- When generating random values in a range, each possible value is equally likely to occur
- The next value from a generator can't be easily predicted, unless you know both the implementation details and the seed
- Patterns are unlikely to emerge

### Generators

A generator is a data structure (`Generator.t('a))` that represents the ability to produce random values of some type `'a`. To run one, you call `Generator.run` with a generator and a seed (see below). `run` returns a value of type `'a` and a new seed, to be used with your next call to `Generator.run`.

In addition be being `run`, generators can also be constructed via functions like `RandomInt.make(~min=30, ~max=50)` (which returns a `Generator.t(int)`).

Generators can also be composed using functions like `map` and `flatMap`, for example:

```reason
open ReludeRandom;

// Here we map an int gnereator into a bool generator, but the
// output boolean is weighted to produce `false` ~70% of the time
let myBoolGenerator =
  RandomInt.make(~min=1, ~max=100)
  |> Generator.map(number => number > 70);
```

### Seeds

In order to run a generator and actually get a value out, you need to construct a seed. This can be as simple as `ReludeRandom.Seed.fromInt(42)`. Constructing a seed this way will cause your generators to always produce the same sequence of values, across runs.

While producing the same sequence of values can be useful for testing, if you're looking for randomness, it's probably because you want your application to behave differently each time it's run. In order to do this, you should construct a seed as a side-effect, based on an `int` that is constantly changing, such as the current time.

The need to produce a different seed each run is common enough that we provide `Seed.init` which has type `Relude.IO.t(Seed.t, Void.t)`. An IO represents a lazy side effect (sort of like a Promise that isn't running yet), which will resolve to a `Seed.t`. IO has an error channel, but since this effect cannot fail, the error type is `Void.t`, which is a type that can't be constructed.

## Differences from Elm Random

While the implementation of the algorithm is basically a direct port from Elm, there are a few key differences in usage:

- Elm has a couple different ways to get a value from a generator. One is more tightly integrated with the Elm Architecture (producing a `Cmd msg`), which doesn't make as much sense in our context. Instead we only have the equivalent of `step`, which we name `Generator.run`.
- Some functions were renamed for consistency with the Relude ecosystem, (e.g. `constant` is `pure` and `andThen` is `flatMap`)
- The official Elm library provides most of the core pieces, while useful "extra" functions live in [a separate elm-community project](https://package.elm-lang.org/packages/elm-community/random-extra/latest). We're in the process of adding many of those directly to Relude Random.
