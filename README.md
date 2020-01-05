# Relude Random

[![GitHub CI](https://img.shields.io/github/workflow/status/reazen/relude-random/CI/master)](https://github.com/reazen/relude-random/actions)
[![test coverage](https://img.shields.io/coveralls/github/reazen/relude-random.svg)](https://coveralls.io/github/reazen/relude-random)
[![npm version](https://img.shields.io/npm/v/relude-random.svg)](https://www.npmjs.com/package/relude-random)
[![license](https://img.shields.io/github/license/reazen/relude-random.svg)](https://github.com/reazen/relude-random/blob/master/LICENSE)

Generate pseudo-random values in ReasonML (in a way that is reproducible and free from side effects). This library implements [PCG random generators](http://www.pcg-random.org/paper.html) and is largely a port of [elm/random](https://package.elm-lang.org/packages/elm/random/latest/) (with [a few differences](https://github.com/reazen/relude-random#differences-from-elm-random))

## Installation

**Install from npm**

```sh
npm install --save relude-random
```

Note that `relude-random` has peer-dependencies on `relude` and `bs-abstract`, both of which can also be installed from npm if your project doesn't already use them.

**Update your bsconfig.json**

```json
{
  "dependencies": [
    "bs-abstract",
    "relude",
    "relude-random"
  ]
}
```

## Understanding Relude Random

If the goal of pure, functional programming is to write functions that always produce the same output, given the same input, without producing any observable side effects, this would seem to be at odds with randomness.

Instead of _true_ randomness, we opt for _generators_ that can be run with a _seed_. Running the same generator with the same seed will always produce the same value. This is "good enough" for many situations where randomness is needed (such as games) because:

- When generating random values in a range, each possible value is equally likely to occur
- The next value from a generator can't be easily predicted, unless you know both the implementation details and the seed
- Sequences of random values are extremely unlikely to repeat

### Generators

A `Generator.t('a)` is a type that represents the ability to produce random values of some type `'a`. To run one, you call `Generator.run` with a generator and a seed (see below). `run` returns a value of type `'a` and a new seed, to be used with your next call to `Generator.run`.

In addition to `run`, generators can also be constructed via functions like `RandomInt.make(~min=30, ~max=50)` (which returns a `Generator.t(int)`).

Generators can also be composed using functions like `map` and `flatMap`, for example:

```reason
open ReludeRandom;

// Here we map an int generator into a bool generator, but the
// output boolean is weighted to produce `false` ~70% of the time
let myBoolGenerator =
  RandomInt.make(~min=1, ~max=100)
  |> Generator.map(number => number > 70);
```

### Seeds

In order to run a generator and actually get a value out, you need to construct a seed. This can be as simple as `ReludeRandom.Seed.fromInt(42)`. Constructing a seed this way will cause your generators to always produce the same values, in the same order, across runs.

While producing the same sequence of values can be useful for testing, if you're looking for randomness, it's probably because you want your application to behave differently each time it's run. In order to do this, you should construct a seed as a side-effect, based on an `int` that is constantly changing, such as the current time.

The need to produce a different seed each run is common enough that we provide `Seed.init` which has type `Relude.IO.t(Seed.t, Void.t)`. An IO represents a lazy side effect (sort of like a Promise that isn't running yet), which will resolve to a `Seed.t`. IO has an error channel, but since this effect cannot fail, the error type is `Void.t`, which is a type that can't be constructed.

## Example

Let's build a generator for local dates (basically a 3-tuple of year, month, day) using the type defined in [Relude Eon](https://github.com/reazen/relude-eon). We'll limit our output to any valid date in the 1900s.

```reason
open ReludeEon;
open ReludeRandom;

let yearGen =
  RandomInt.make(~min=1900, ~max=1999) |> Generator.map(Year.fromInt);

// We could also use `Generator.fromBoundedEnum` here to save ourselves from
// having to type out all the months, but for a small example this is clearer.
let monthGen =
  Generator.uniform(
    Month.Jan,
    [Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec]
  );

// We can combine the year and month generators using `map2` because they don't
// depend on each other. Creating a new generator for a tuple like this can also
// be done using `Generator.tuple2`.
let yearMonthGen =
  Generator.map2((year, month) => (year, month), yearGen, monthGen);

// Choosing a random day-of-month _does_ depend on the output of the previous
// year-month generator, so we use flatMap
let localDateGen =
  yearMonthGen
  |> Generator.flatMap(((year, month)) => {
       let yearInt = Year.getYear(year);
       let isLeapYear = Year.isLeapYear(year);
       let max = Month.totalDays(isLeapYear, month);

       RandomInt.make(~min=1, ~max)
       |> Generator.map(day => LocalDate.makeClamped(yearInt, month, day));
  });

// Run the generator with a fixed seed, producing a date like (1961, Jun, 30)
let (date, _nextSeed) = Generator.run(localDateGen, Seed.fromInt(-761313));

// More likely, you'd want to use an unknown seed, which gets you into the world
// of IO for tracking side effects. If you're using a React hook to manage your
// application state, [relude-reason-react][1] has helpers for working with IO.
//
// [1] https://github.com/reazen/relude-reason-react
Seed.init
|> IO.map(Generator.run(localDateGen))
|> IO.map(((date, nextSeed)) => feedIntoReducer(date, nextSeed))
|> IO.unsafeRunAsync(/*...*/);
```

## Differences from Elm Random

While the implementation of the algorithm is basically a direct port from Elm, there are a few key differences in usage:

**Different Random Values**

The algorithm is based on bit-shifting, and because ints in Elm match the range of JS numbers (`2^53`), while Bucklescript uses 32-bit ints, the overflow happens at different values, leading to different results.

While random values are still evenly distributed and hard to predict, this difference means that code ported from Elm will generate different values with Relude Random, even when the same initial seed is used.

**Generating Values**

Elm has a couple different ways to get a value from a generator. One is more tightly integrated with the Elm Architecture (producing a `Cmd msg`), which doesn't make as much sense in our context. Instead we only have the equivalent of Elm's `step` function, which we've named `Generator.run`.

**Inclusion of More Helpers**

The official Elm library provides most of the core pieces, but useful "extra" functions live in [a separate elm-community project](https://package.elm-lang.org/packages/elm-community/random-extra/latest). We're in the process of adding many of those directly to Relude Random.

**Naming**

Types and functions were renamed for consistency with the Relude ecosystem:

- Generator and Seed types live in their own modules (as Generator.t and Seed.t)
- `Random.initialSeed` is `Seed.fromInt`
- `Random.constant` is `Generator.pure`
- `Random.andThen` is `Generator.flatMap`
- `Random.step` is `Generator.run`

Additionally, Elm's `random-extra` library has `Char` generators, but the `char` type in Reason is much more limited, so we use `string` even for single characters.
