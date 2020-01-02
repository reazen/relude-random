open Jest;
open Expect;

open Relude.Globals;
open ReludeRandom;

type weekday =
  | Sun
  | Mon
  | Tue
  | Wed
  | Thu
  | Fri
  | Sat;

module WeekdayOrd = {
  type t = weekday;
  let eq = (a: t, b: t) => a == b;
  let compare = (a: t, b: t) => compare(a, b) |> Relude.Ordering.fromInt;
};

// describe("Seed", () => {
//   test("construct a seed", () => {
//     let v = Seed.fromInt(42);
//     expect(v) |> toEqual(Seed.Seed(1266345812, 1013904223));
//   });
//   test("peel an int from a seed", () => {
//     let v = Seed.peel(Seed.fromInt(42));
//     expect(v) |> toEqual(1298916341);
//   });
// });

/**
 * Testing randomness seems like a real challenge... Maybe property-based tests
 * would be better? I at least want some sanity checks to make sure the API
 * works as advertised and produces evenly-distributed values.
 */
describe("Generator", () => {
  test("int with constant seed", () => {
    Generator.int(~min=1, ~max=100)
    |> Generator.run(_, Seed.fromInt(42))
    |> fst
    |> expect
    |> toEqual(39)
  })
});

describe("RandomList", () => {
  test("list with fixed size", () =>
    RandomList.make(~length=7, Generator.int(~min=-10, ~max=10))
    |> Generator.run(_, Seed.fromInt(4))
    |> fst
    |> expect
    |> toEqual([(-8), (-4), (-5), (-4), 4, 7, 4])
  );

  test("list with variable size", () =>
    RandomList.makeRandomLength(
      ~minLength=3,
      ~maxLength=19,
      Generator.float(~min=0.0, ~max=1.0),
    )
    |> Generator.run(_, Seed.fromInt(888888))
    |> fst
    |> List.length
    |> expect
    |> toEqual(7)
  );

  test("list with negative size", () =>
    RandomList.makeRandomLength(
      ~minLength=-20,
      ~maxLength=-10,
      Generator.int(~min=0, ~max=2),
    )
    |> Generator.run(_, Seed.fromInt(1))
    |> fst
    |> expect
    |> toEqual([])
  );

  let collectCounts =
      (type a, ord: (module BsAbstract.Interface.ORD with type t = a), xs) => {
    module Ord = (val ord);
    module M = Relude.Map.WithOrd(Ord);
    List.foldLeft(
      (acc, x) => M.(set(x, Option.fold(1, v => v + 1, get(x, acc)), acc)),
      M.make(),
      xs,
    )
    |> M.toList;
  };

  test("values are distributed evenly", () => {
    let seed = Seed.fromInt(987654);
    let diceGen = RandomInt.make(~min=1, ~max=6);
    RandomList.make(~length=60000, diceGen)
    |> Generator.run(_, seed)
    |> fst
    |> collectCounts((module Int.Ord))
    |> expect
    |> toEqual([
         (1, 10087),
         (2, 9968),
         (3, 10053),
         (4, 9815),
         (5, 9907),
         (6, 10170),
       ]);
  });

  test("values are distributed evenly (different seed)", () => {
    let seed = Seed.fromInt(71);
    let diceGen = RandomInt.make(~min=1, ~max=6);
    RandomList.make(~length=60000, diceGen)
    |> Generator.run(_, seed)
    |> fst
    |> collectCounts((module Int.Ord))
    |> expect
    |> toEqual([
         (1, 9789),
         (2, 10145),
         (3, 10095),
         (4, 9938),
         (5, 9986),
         (6, 10047),
       ]);
  });

  test("weighted values break down as expected", () => {
    let seed = Seed.fromInt(-744982);
    let weekdayGen =
      Generator.weighted(
        (10.0, Sun),
        [
          (1.0, Mon),
          (1.0, Tue),
          (1.0, Wed),
          (1.0, Thu),
          (2.0, Fri),
          (4.0, Sat),
        ],
      );

    RandomList.make(~length=20000, weekdayGen)
    |> Generator.run(_, seed)
    |> fst
    |> collectCounts((module WeekdayOrd))
    |> expect
    |> toEqual([
         (Sun, 9890),
         (Mon, 1048),
         (Tue, 969),
         (Wed, 970),
         (Thu, 964),
         (Fri, 2007),
         (Sat, 4152),
       ]);
  });
});
