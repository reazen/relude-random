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
