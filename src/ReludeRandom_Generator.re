open Relude.Globals;

module Seed = ReludeRandom_Seed;

type t('a) =
  | Generator(Seed.t => ('a, Seed.t));

let run = (Generator(f), seed) => f(seed);

let map = (f, Generator(genA)) =>
  Generator(
    seed0 => {
      let (a, seed1) = genA(seed0);
      (f(a), seed1);
    },
  );

let apply = (Generator(genF), Generator(genA)) =>
  Generator(
    seed0 => {
      let (f, seed1) = genF(seed0);
      let (a, seed2) = genA(seed1);
      (f(a), seed2);
    },
  );

let pure = a => Generator(seed => (a, seed));

let flatMap = (f, Generator(genA)) =>
  Generator(
    seed0 => {
      let (a, seed1) = genA(seed0);
      let Generator(genB) = f(a);
      genB(seed1);
    },
  );

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
          x < threshold ? accountForBias(seedN) : (x mod range + lo, seedN);
        };
        accountForBias(seed0);
      };
    },
  );

let weighted = (first, rest) => {
  let normalize = ((weight, _)) => abs_float(weight);
  let total = normalize(first) +. List.Float.sum(List.map(normalize, rest));

  let rec getByWeight = ((weight, value), rest, countdown) =>
    switch (rest) {
    | [x, ...xs] when countdown > abs_float(weight) =>
      getByWeight(x, xs, countdown -. abs_float(weight))
    | _ => value
    };

  float(~min=0.0, ~max=total) |> map(getByWeight(first, rest));
};

module Functor = {
  type nonrec t('a) = t('a);
  let map = map;
};

module Apply = {
  include Functor;
  let apply = apply;
};

module Applicative = {
  include Apply;
  let pure = pure;
};

module Monad = {
  include Applicative;
  let flat_map = (a, f) => flatMap(f, a);
};

include Relude.Extensions.Functor.FunctorExtensions(Functor);
include Relude.Extensions.Apply.ApplyExtensions(Apply);
include Relude.Extensions.Monad.MonadExtensions(Monad);
