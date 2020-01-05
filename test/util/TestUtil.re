open Relude.Globals;
open ReludeRandom;

module Weekday = {
  type t =
    | Sun
    | Mon
    | Tue
    | Wed
    | Thu
    | Fri
    | Sat;

  let fromEnum =
    fun
    | Sun => 0
    | Mon => 1
    | Tue => 2
    | Wed => 3
    | Thu => 4
    | Fri => 5
    | Sat => 6;

  let toEnum =
    fun
    | 0 => Some(Sun)
    | 1 => Some(Mon)
    | 2 => Some(Tue)
    | 3 => Some(Wed)
    | 4 => Some(Thu)
    | 5 => Some(Fri)
    | 6 => Some(Sat)
    | _ => None;

  let eq = (a: t, b: t) => a == b;
  let compare = (a: t, b: t) => compare(a, b) |> Ordering.fromInt;

  let succ = v => toEnum(fromEnum(v) + 1);
  let pred = v => toEnum(fromEnum(v) - 1);

  let cardinality = 7;

  let bottom = Sun;
  let top = Sat;
};

let collectCounts =
    (type a, ord: (module BsAbstract.Interface.ORD with type t = a), xs) => {
  module Ord = (val ord);
  module M = Map.WithOrd(Ord);
  List.foldLeft(
    (acc, x) => M.(set(x, Option.fold(1, v => v + 1, get(x, acc)), acc)),
    M.make(),
    xs,
  )
  |> M.toList;
};

let testDistribution =
    (
      type a,
      ord: (module BsAbstract.Interface.ORD with type t = a),
      ~samples=10000,
      gen: Generator.t(a),
      seed,
    ) => {
  module Ord = (val ord);
  RandomList.make(~length=samples, gen)
  |> Generator.run(_, seed)
  |> fst
  |> collectCounts((module Ord));
};

let all = (~samples=10000, pred, gen, seed) =>
  RandomList.make(~length=samples, gen)
  |> Generator.run(_, seed)
  |> fst
  |> List.all(pred);

// this is sort of like a poor man's version of property-based testing. we
// generate ~10000 truly random values, then assert that each passes some
// predicate. hopefully none ever fail, because we don't have great reporting
// that would allow us to reproduce failures.
let testAllRandomSeed = (~samples=?, msg, pred, gen) =>
  Jest.testAsync(msg, onDone =>
    Seed.init
    |> IO.map(all(~samples?, pred, gen))
    |> IO.unsafeRunAsync(
         fun
         | Ok(v) => Jest.Expect.(expect(v) |> toEqual(true)) |> onDone
         | Error(x) => Void.absurd(x),
       )
  );
