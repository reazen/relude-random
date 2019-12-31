module Seed: {
  type t;

  /**
   * Produce a seed from a known integer. This can be used to start a sequence
   * of "random" values that will be reproducible across runs.
   *
   * This is valuable for testing, or if you want to create a pattern exactly
   * one time. However, in many cases you'll want each run to be different, in
   * which case you can create a seed from the current time, using `init`.
   *
   * ```reason
   * let mySeed: Seed.t = Seed.fromInt(42);
   * ```
   */
  let fromInt: int => t;

  /**
   * Produce a "random" seed based on the current time. Effectively, this seed
   * will be different each time the program is run, making each run unique.
   */
  let init: Relude.IO.t(t, Relude.Void.t);
};

module Generator: {
  type t('a);

  module Functor: BsAbstract.Interface.FUNCTOR with type t('a) = t('a);

  module Apply: BsAbstract.Interface.APPLY with type t('a) = t('a);

  module Applicative:
    BsAbstract.Interface.APPLICATIVE with type t('a) = t('a);

  module Monad: BsAbstract.Interface.MONAD with type t('a) = t('a);
  /**
   * Run a generator with the provided seed, getting back a tuple of the random
   * value and the next seed.
   *
   * This function is pure, as calling the generator function with the same seed
   * will always return the same "random" value and the same next seed.
   */
  let run: (t('a), Seed.t) => ('a, Seed.t);

  /**
   * Map a given `Generator.t('a)` to a `Generator.t('b)` using the provided
   * function. This is the foundation for building more comlpex generators from
   * the ones provided in this library.
   *
   * ```reason
   * // construct a `Generator.t(bool)` that will be true ~70% of the time
   * let weightedBool: Generator.t(bool) =
   *   Generator.int(~min=1, ~max=100)
   *   |> Generator.map(v => v < 70);
   */
  let map: ('a => 'b, t('a)) => t('b);

  /**
   * Combine two generators, where the first generates a function which will be
   * applied to the value generated by the second. This may not look obviously
   * useful, but it's the foundation for functions like `map2` and friends.
   *
   * It's important to note that unlike many `apply` functions (such as the ones
   * for `option` and `IO.t`), `apply` for generators is a sequential operation,
   * where each subsequent generator depends on the seed produced by the one
   * before it.
   */
  let apply: (t('a => 'b), t('a)) => t('b);

  /**
   * Construct a generator from the provided value. When run, the generator will
   * always resolve to this value.
   */
  let pure: 'a => t('a);

  /**
   * Chain together multiple generators. This is useful when constructing a
   * generator relies on the output of a previous generator, for example
   * building a list generator of a random length.
   */
  let flatMap: ('a => t('b), t('a)) => t('b);

  /**
   * Construct a generator for floating point numbers between some min and max.
   *
   * ```reason
   * let probability = Generator.float(~min=0.0, ~max=1.0);
   * let (value, nextSeed) = Seed.fromInt(42) |> step(probability);
   * ```
   */
  let float: (~min: float, ~max: float) => t(float);

  /**
   * Construct a generator that will produce int values between the provided min
   * and max.
   */
  let int: (~min: int, ~max: int) => t(int);

  /**
   * `map` with arguments flipped.
   */
  let flipMap: (t('a), 'a => 'b) => t('b);

  /**
   * Build a generator that relies on the result of two smaller generators. This
   * is a useful pattern for combining generators that don't strictly depend on
   * each other. If one generator instead depends on the output of the other,
   * see `flatMap` instead.
   *
   * Note that internally, the second generator _will_ depend on the output of
   * the first, as the first produces a new seed, which is fed into the second.
   * In practice, this means that the order in which you pass generators to this
   * function is significant.
   *
   * ```reason
   * type point = {x: int, y: int};
   * let pointGenerator =
   *   Generator.map2(
   *     (x, y) => {x, y},
   *     RandomInt.make(~min=0, ~max=100),
   *     RandomInt.make(~min=0, ~max=20),
   *   );
   *
   * Generator.run(pointGenerator, mySeed); // e.g. {x: 72, y: 4}
   * ```
   */
  let map2: (('a, 'b) => 'c, t('a), t('b)) => t('c);

  /**
   * Build a new generator that depends on the values produced by three smaller
   * generators. For a more complete example, see the docs for `map2`.
   */
  let map3: (('a, 'b, 'c) => 'd, t('a), t('b), t('c)) => t('d);

  /**
   * Build a new generator that depends on the values produced by four smaller
   * generators. For a more complete example, see the docs for `map2`.
   */
  let map4: (('a, 'b, 'c, 'd) => 'e, t('a), t('b), t('c), t('d)) => t('e);

  /**
   * Build a new generator that depends on the values produced by five smaller
   * generators. For a more complete example, see the docs for `map2`.
   *
   * Our `mapN` helpers end with 5. If you're looking for more you have a few
   * options. First, you can chain together `map` and `apply` functions an
   * arbitrary number of times to achieve this effect (see [this blog post][1]).
   * Alternatively, you could just chain things together with flatMap, or wait
   * for the upcoming [let+ syntax][2] to make its way into Reason.
   *
   * [1] https://andywhite.xyz/posts/2019-11-07-a-laymans-guide-to-applicatives-in-reasonml/
   * [2] https://jobjo.github.io/2019/04/24/ocaml-has-some-new-shiny-syntax.html
   */
  let map5:
    (('a, 'b, 'c, 'd, 'e) => 'f, t('a), t('b), t('c), t('d), t('e)) =>
    t('f);

  /**
   * Compose two functions that produce generators, resulting in a function from
   * the simple input to the final generator. `composeKleisli` is often used in
   * its infix form (>=>). For example:
   *
   * ```reason
   * let (>=>) = Generator.composeKleisli;
   * let charList = length =>
   *   RandomList.make(~length, RandomChar.latinLowercase);
   *
   * // `stringOfRandomLength` is a function that takes a max length and returns
   * // a generator for random strings with a length between 0 and that max
   * let stringOfRandomLength =
   *   RandomInt.fromZeroTo >=> charList |> Generator.map(List.String.join);
   *
   * // produces values like "rzacx" or ""
   * let latinString = stringOfRandomLength(8) |> Generator.run(_, mySeed);
   * ```
   */
  let composeKleisli: ('a => t('b), 'b => t('c), 'a) => t('c);

  /**
   * Kleisli composition with the functions flipped, often written as (<=<). The
   * docs for `composeKleisli` explain the usage more clearly, with examples.
   */
  let flipComposeKleisli: ('b => t('c), 'a => t('b), 'a) => t('c);

  /**
   * Turn a generator of a generator into a single layer of generator. This is
   * probably not often needed, since you're more likely to flatMap and never
   * end up with the nested generators in the first place.
   */
  let flatten: t(t('a)) => t('a);
};

module RandomInt: {
  /**
   * Construct a generator that will produce int values between the provided min
   * and max.
   */
  let make: (~min: int, ~max: int) => Generator.t(int);

  /**
   * A generator that can produce any signed 32-bit integer value.
   */
  let any: Generator.t(int);

  /**
   * An int generator that can produce any integer between 1 and the upper limit
   * for signed 32-bit integers (`max_int`).
   */
  let anyPositive: Generator.t(int);

  /**
   * An int generator where the upper bound is -1 and the lower bound is
   * `min_int` (the lowest possible value for 32-bit signed ints).
   */
  let anyNegative: Generator.t(int);

  /**
   * Given an int, returns a generator that will produce values at least 1 plus
   * the provided value, with an upper bound of `max_int`.
   */
  let greaterThan: int => Generator.t(int);

  /**
   * Given an int, returns a generator that will produce values at most 1 less
   * than the provided value, with a lower bound of `min_int`.
   */
  let lessThan: int => Generator.t(int);

  /**
   * Produce a random int between zero and the provided number. The value can be
   * positive or negative, e.g.
   *
   * ```reason
   * fromZeroTo(-7); // generator that produces values between -7...0
   * fromZeroTo(4); // generator that produces values between 0...4
   * ```
   */
  let fromZeroTo: int => Generator.t(int);
};

module RandomList: {
  /**
   * Given a generator of some type, produce a generator for a list of values of
   * that type. The random list will have a fixed length, using the provided
   * `length` value. Negative lengths will result in an empty list.
   *
   * ```reason
   * RandomList.make(~length=4, RandomInt.make(~min=1, ~max=10))
   * |> Generator.run(_, mySeed); // [2, 9, 7, 3]
   * ```
   */
  let make: (~length: int, Generator.t('a)) => Generator.t(list('a));

  /**
   * Construct a list with random values using the provided generator, with a
   * random length between the provided min and max. If the minLength is less
   * than 0, it will be clamped to 0 before choosing the length
   */
  let makeRandomLength:
    (~minLength: int, ~maxLength: int, Generator.t('a)) =>
    Generator.t(list('a));
};
