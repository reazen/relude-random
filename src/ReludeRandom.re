open Relude.Globals;

// Bitwise operations are already globally available via OCaml's Pervasives, but
// I find it challenging to keep track of infix `lsl`, so I'm aliasing them
// here, which also makes it slightly easier to translate from Elm's named
// Bitwise operations.
module Bitwise = {
  let and_ = (a, b) => a land b;
  let or_ = (a, b) => a lor b;
  let xor = (a, b) => a lxor b;
  let shiftRightZeroFill = (a, b) => b lsr a;
};

module Seed = {
  type t =
    | Seed(int, int);

  // produce the next seed, given a current seed
  let next = (Seed(state0, inc)) =>
    Seed((state0 * 1664525 + inc) lsr 0, inc);

  // produce a pseudorandom int from a seed
  let peel = (Seed(state, _)) => {
    let vshift = Bitwise.shiftRightZeroFill(state lsr 28 + 4, state);
    let word = state lxor vshift * 277803737;
    let shift22 = Bitwise.shiftRightZeroFill(22, word);
    Bitwise.shiftRightZeroFill(0, shift22 lxor word);
    // let word = 277803737 * (state lxor state lsr (state + 4 lsr 28));
    // (word lsr 22 lxor word) lsr 0;
  };

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
  let fromInt = x => {
    let Seed(state1, inc) = next(Seed(0, 1013904223));
    let state2 = (state1 + x) lsr 0;
    next(Seed(state2, inc));
  };

  /**
   * Produce a "random" seed based on the current time. Effectively, this seed
   * will be different each time the program is run, making each run unique.
   */
  let init: IO.t(t, Void.t) =
    IO.suspendWithVoid(() => Js.Date.now())
    |> IO.map(ms => fromInt(int_of_float(ms)));
};

module Generator = {
  type t('a) =
    | Generator(Seed.t => ('a, Seed.t));

  /**
   * Run a generator with the provided seed, getting back a tuple of the random
   * value and the next seed.
   *
   * This function is pure, as calling the generator function with the same seed
   * will always return the same "random" value and the same next seed.
   */
  let step = (Generator(f), seed) => f(seed);

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
  let map: 'a 'b. ('a => 'b, t('a)) => t('b) =
    (f, Generator(genA)) =>
      Generator(
        seed0 => {
          let (a, seed1) = genA(seed0);
          (f(a), seed1);
        },
      );

  /**
   * Construct a Generator that always returns the same value, by providing that
   * value.
   */
  let pure: 'a. 'a => t('a) = a => Generator(seed => (a, seed));

  /**
   * Chain together multiple generators. This is useful when constructing a
   * generator relies on the output of a previous generator, for example
   * building a list generator of a random length.
   */
  let flatMap: 'a 'b. ('a => t('b), t('a)) => t('b) =
    (f, Generator(genA)) =>
      Generator(
        seed0 => {
          let (a, seed1) = genA(seed0);
          let Generator(genB) = f(a);
          genB(seed1);
        },
      );

  /**
   * Construct a generator for floating point numbers between some min and max.
   *
   * ```reason
   * let probability = Generator.float(~min=0.0, ~max=1.0);
   * let (value, nextSeed) = Seed.fromInt(42) |> step(probability);
   * ```
   */
  let float = (~min: float, ~max: float): t(float) =>
    Generator(
      seed0 => {
        let seed1 = Seed.next(seed0);
        let n0 = Seed.peel(seed0);
        let n1 = Seed.peel(seed1);

        // get a uniformly distributed IEEE-754 double between 0.0 and 1.0
        let hi = float_of_int(0x03FFFFFF land n0) *. 1.0;
        let lo = float_of_int(0x07FFFFFF land n1) *. 1.0;

        // magic constants are 2^27 and 2^53
        let v = (hi *. 134217728.0 +. lo) /. 9007199254740992.0;

        // scale it to the min/max range provided
        let range = abs_float(max -. min);
        let scaled = v *. range +. min;

        (scaled, Seed.next(seed1));
      },
    );

  let int = (~min: int, ~max: int): t(int) =>
    Generator(
      seed0 => {
        // make sure the provided min isn't greater than the provided max
        let (lo, hi) = min < max ? (min, max) : (max, min);
        let range = hi - lo + 1;

        // fast path for power of 2
        if ((range - 1) land range == 0) {
          let value = (range - 1) land Seed.peel(seed0) lsr 0;
          (value + lo, Seed.next(seed0));
        } else {
          // `mod` in OCaml works similarly to `remainderBy` in Elm... this is
          // important when negative numbers get involved: `-5 mod 4 == -1`
          let threshold = (- range lsr 0 mod range) lsr 0;

          // recursively ensure the seed is within our threshold, but in
          // practice this almost never recurses
          let rec accountForBias = (seed: Seed.t): (int, Seed.t) => {
            let x = Seed.peel(seed);
            let seedN = Seed.next(seed);
            x < threshold
              ? accountForBias(seedN) : (x mod range + lo, seedN);
          };
          accountForBias(seed0);
        };
      },
    );
};

module RandomInt = {
  let inRange = Generator.int;

  let any = inRange(~min=min_int, ~max=max_int);

  let anyPositive = inRange(~min=1, ~max=max_int);

  let anyNegative = inRange(~min=min_int, ~max=-1);

  let greaterThan = n => inRange(~min=n + 1, ~max=max_int);

  let lessThan = n => inRange(~min=min_int, ~max=n - 1);
};

module RandomList = {
  let rec listHelp:
    (list('a), int, Seed.t => ('a, Seed.t), Seed.t) => (list('a), Seed.t) =
    (acc, n, gen, seed) =>
      if (n < 1) {
        (acc, seed);
      } else {
        let (value, newSeed) = gen(seed);
        listHelp([value, ...acc], n - 1, gen, newSeed);
      };

  /**
   * Construct a list with random values, using the provided generator. The
   * length is fixed using the provided `length` parameter. Negative lengths
   * will result in empty lists.
   *
   * ```reason
   * // e.g. ([912324, 7, 4114674, 12], seed)
   * let (myValue, nextSeed) =
   *   RandomList.make(~length=4, RandomInt.anyPositive)
   *   |> Generator.run(_, someSeed);
   */
  let make = (~length: int, Generator.Generator(gen)) =>
    Generator.Generator(seed => listHelp([], length, gen, seed));

  /**
   * Construct a list with random values using the provided generator, with a
   * random length between the provided min and max. If the minLength is less
   * than 0, it will be clamped to 0 before choosing the length
   */
  let makeRandomLength = (~minLength: int, ~maxLength: int, gen) =>
    Generator.int(~min=Int.max(minLength, 0), ~max=maxLength)
    |> Generator.flatMap(length => make(~length, gen));
};
